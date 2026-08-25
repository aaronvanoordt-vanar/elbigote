/* ==========================================================================
   El Bigote Coffee & Waffles — comportamiento de la web
   Sin dependencias. Todo degrada con elegancia si algo falla.
   ========================================================================== */
(function () {
  'use strict';

  var $  = function (sel, ctx) { return (ctx || document).querySelector(sel); };
  var $$ = function (sel, ctx) { return Array.prototype.slice.call((ctx || document).querySelectorAll(sel)); };
  var reduceMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  /* ── Configuración del negocio ──────────────────────────────────────── */
  var HORARIO = { abre: 9 * 60, cierra: 21 * 60 + 15 };  // minutos desde medianoche
  var ZONA = 'America/Lima';
  var DIAS = ['domingo', 'lunes', 'martes', 'miércoles', 'jueves', 'viernes', 'sábado'];

  /* ── 1. Año en el pie ───────────────────────────────────────────────── */
  $$('[data-year]').forEach(function (el) { el.textContent = String(new Date().getFullYear()); });

  /* ── 2. Estado abierto / cerrado en hora de Lima ─────────────────────
     Perú no aplica horario de verano, pero usamos Intl para no depender
     de la zona horaria del visitante.                                    */
  function horaLima() {
    try {
      var partes = new Intl.DateTimeFormat('en-US', {
        timeZone: ZONA, hour12: false,
        weekday: 'short', hour: '2-digit', minute: '2-digit'
      }).formatToParts(new Date());
      var v = {};
      partes.forEach(function (p) { v[p.type] = p.value; });
      var mapa = { Sun: 0, Mon: 1, Tue: 2, Wed: 3, Thu: 4, Fri: 5, Sat: 6 };
      var h = parseInt(v.hour, 10);
      if (h === 24) h = 0;                      // algunos motores devuelven 24
      return { dia: mapa[v.weekday], minutos: h * 60 + parseInt(v.minute, 10) };
    } catch (e) {
      var d = new Date();
      return { dia: d.getDay(), minutos: d.getHours() * 60 + d.getMinutes() };
    }
  }

  function hhmm(min) {
    var h = Math.floor(min / 60), m = min % 60;
    return h + ':' + (m < 10 ? '0' + m : m);
  }

  function pintarEstado() {
    var ahora = horaLima();
    var estado, texto;

    if (ahora.minutos >= HORARIO.abre && ahora.minutos < HORARIO.cierra) {
      var quedan = HORARIO.cierra - ahora.minutos;
      if (quedan <= 60) { estado = 'soon'; texto = 'Cerramos en ' + quedan + ' min'; }
      else { estado = 'open'; texto = 'Abierto ahora · hasta las ' + hhmm(HORARIO.cierra); }
    } else {
      estado = 'closed';
      texto = ahora.minutos < HORARIO.abre
        ? 'Cerrado · abrimos hoy a las ' + hhmm(HORARIO.abre)
        : 'Cerrado · abrimos mañana a las ' + hhmm(HORARIO.abre);
    }

    $$('[data-status]').forEach(function (el) {
      el.setAttribute('data-state', estado);
      var t = $('[data-status-text]', el);
      if (t) t.textContent = texto;
    });

    $$('.hours__row').forEach(function (row) {
      row.setAttribute('data-today', String(Number(row.getAttribute('data-day')) === ahora.dia));
    });
  }
  pintarEstado();
  setInterval(pintarEstado, 60000);

  /* ── 3. Cabecera fija ───────────────────────────────────────────────── */
  var header = $('#siteHeader');
  var actionBar = $('#actionBar');
  var ultimoScroll = 0;

  function alScroll() {
    var y = window.scrollY || window.pageYOffset;
    if (header) header.setAttribute('data-stuck', String(y > 40));
    // La barra de acción aparece una vez pasada la portada
    if (actionBar) actionBar.setAttribute('data-show', String(y > window.innerHeight * 0.6));
    ultimoScroll = y;
  }
  var ticking = false;
  window.addEventListener('scroll', function () {
    if (!ticking) {
      window.requestAnimationFrame(function () { alScroll(); ticking = false; });
      ticking = true;
    }
  }, { passive: true });
  alScroll();

  /* ── 4. Panel de navegación móvil ───────────────────────────────────── */
  var burger = $('#burger');
  var drawer = $('#drawer');

  function abrirDrawer(abrir) {
    if (!burger || !drawer) return;
    drawer.setAttribute('data-open', String(abrir));
    burger.setAttribute('aria-expanded', String(abrir));
    burger.setAttribute('aria-label', abrir ? 'Cerrar menú de navegación' : 'Abrir menú de navegación');
    document.body.style.overflow = abrir ? 'hidden' : '';
    if (abrir) {
      // El panel entra con una transición de visibilidad: enfocar antes no surte efecto.
      setTimeout(function () {
        var primero = $('.drawer__link', drawer);
        if (primero && drawer.getAttribute('data-open') === 'true') primero.focus();
      }, 320);
    }
  }

  if (burger) {
    burger.addEventListener('click', function () {
      abrirDrawer(drawer.getAttribute('data-open') !== 'true');
    });
  }
  $$('.drawer__link, .drawer__foot a').forEach(function (a) {
    a.addEventListener('click', function () { abrirDrawer(false); });
  });

  /* ── 5. Filtros de la carta ─────────────────────────────────────────── */
  var grupos = $$('.menu-group');
  var vacio = $('[data-menu-empty]');
  var buscador = $('#menuSearch');
  var filtroActivo = 'all';

  // Contador de platos por sección
  grupos.forEach(function (g) {
    var n = $$('.menu-item', g).length;
    var c = $('[data-count]', g);
    if (c) c.textContent = n + (n === 1 ? ' plato' : ' platos');
  });

  function normalizar(s) {
    return (s || '').toLowerCase().normalize('NFD').replace(/[\u0300-\u036f]/g, '');
  }

  function aplicarFiltros() {
    var q = normalizar(buscador ? buscador.value.trim() : '');
    var visiblesTotal = 0;

    grupos.forEach(function (grupo) {
      var cats = grupo.getAttribute('data-cat') || '';
      var pasaCat = filtroActivo === 'all' || cats.indexOf(filtroActivo) !== -1;
      var visiblesGrupo = 0;

      $$('.menu-item', grupo).forEach(function (item) {
        var texto = normalizar(item.getAttribute('data-name') + ' ' + item.textContent);
        var pasaTexto = !q || texto.indexOf(q) !== -1;
        var mostrar = pasaCat && pasaTexto;
        item.hidden = !mostrar;
        if (mostrar) visiblesGrupo++;
      });

      grupo.hidden = visiblesGrupo === 0;
      visiblesTotal += visiblesGrupo;

      var c = $('[data-count]', grupo);
      if (c) c.textContent = visiblesGrupo + (visiblesGrupo === 1 ? ' plato' : ' platos');
    });

    if (vacio) vacio.hidden = visiblesTotal > 0;
  }

  $$('.tab').forEach(function (tab) {
    tab.addEventListener('click', function () {
      $$('.tab').forEach(function (t) { t.setAttribute('aria-selected', 'false'); });
      tab.setAttribute('aria-selected', 'true');
      filtroActivo = tab.getAttribute('data-filter');
      aplicarFiltros();
    });
  });

  if (buscador) {
    var debounce;
    buscador.addEventListener('input', function () {
      clearTimeout(debounce);
      debounce = setTimeout(aplicarFiltros, 120);
    });
  }

  /* ── 6. Preguntas frecuentes ────────────────────────────────────────── */
  $$('.faq__item').forEach(function (item) {
    var boton = $('.faq__q', item);
    if (!boton) return;
    boton.addEventListener('click', function () {
      var abierto = item.getAttribute('data-open') === 'true';
      item.setAttribute('data-open', String(!abierto));
      boton.setAttribute('aria-expanded', String(!abierto));
    });
  });

  /* ── 7. Fotos: si un archivo falta, dejamos ver el marcador de marca ── */
  $$('img[data-photo]').forEach(function (img) {
    function fallar() { img.hidden = true; }
    img.addEventListener('error', fallar);
    // Si ya falló antes de que registráramos el listener
    if (img.complete && img.naturalWidth === 0) fallar();
  });

  /* ── 8. Visor de galería ────────────────────────────────────────────── */
  var lightbox = $('#lightbox');
  var lbImg = $('#lightboxImg');
  var lbCap = $('#lightboxCap');
  var botonesFoto = $$('[data-lightbox]');
  var fotos = botonesFoto.map(function (b) {
    var img = $('img', b);
    return { src: img ? img.getAttribute('src') : '', alt: img ? img.getAttribute('alt') : '' };
  });
  var indice = 0;
  var origenFoco = null;

  function mostrarFoto(i) {
    if (!fotos.length) return;
    indice = (i + fotos.length) % fotos.length;
    var f = fotos[indice];
    lbImg.setAttribute('src', f.src);
    lbImg.setAttribute('alt', f.alt);
    lbCap.innerHTML = '<b>' + (indice + 1) + ' / ' + fotos.length + '</b> · ' + f.alt;
  }

  function abrirVisor(i) {
    if (!lightbox) return;
    origenFoco = document.activeElement;
    mostrarFoto(i);
    lightbox.setAttribute('data-open', 'true');
    document.body.style.overflow = 'hidden';
    var cerrar = $('[data-lb-close]', lightbox);
    if (cerrar) cerrar.focus();
  }

  function cerrarVisor() {
    if (!lightbox) return;
    lightbox.setAttribute('data-open', 'false');
    document.body.style.overflow = '';
    if (origenFoco && origenFoco.focus) origenFoco.focus();
  }

  botonesFoto.forEach(function (boton, i) {
    boton.addEventListener('click', function () { abrirVisor(i); });
  });

  if (lightbox) {
    var cerrarBtn = $('[data-lb-close]', lightbox);
    var prevBtn = $('[data-lb-prev]', lightbox);
    var nextBtn = $('[data-lb-next]', lightbox);
    if (cerrarBtn) cerrarBtn.addEventListener('click', cerrarVisor);
    if (prevBtn) prevBtn.addEventListener('click', function () { mostrarFoto(indice - 1); });
    if (nextBtn) nextBtn.addEventListener('click', function () { mostrarFoto(indice + 1); });
    lightbox.addEventListener('click', function (e) { if (e.target === lightbox) cerrarVisor(); });

    // Mantiene el foco dentro del visor mientras está abierto
    lightbox.addEventListener('keydown', function (e) {
      if (e.key !== 'Tab') return;
      var focos = $$('button', lightbox).filter(function (b) { return b.offsetParent !== null; });
      if (!focos.length) return;
      var primero = focos[0], ultimo = focos[focos.length - 1];
      if (e.shiftKey && document.activeElement === primero) { e.preventDefault(); ultimo.focus(); }
      else if (!e.shiftKey && document.activeElement === ultimo) { e.preventDefault(); primero.focus(); }
    });
  }

  document.addEventListener('keydown', function (e) {
    if (lightbox && lightbox.getAttribute('data-open') === 'true') {
      if (e.key === 'Escape') cerrarVisor();
      if (e.key === 'ArrowLeft') mostrarFoto(indice - 1);
      if (e.key === 'ArrowRight') mostrarFoto(indice + 1);
      return;
    }
    if (e.key === 'Escape' && drawer && drawer.getAttribute('data-open') === 'true') {
      abrirDrawer(false);
      if (burger) burger.focus();
    }
  });

  /* ── 9. Aparición al hacer scroll ───────────────────────────────────── */
  var animables = $$('.reveal, .reveal-stagger');
  if (reduceMotion || !('IntersectionObserver' in window)) {
    animables.forEach(function (el) { el.setAttribute('data-visible', 'true'); });
  } else {
    var observador = new IntersectionObserver(function (entradas) {
      entradas.forEach(function (entrada) {
        if (!entrada.isIntersecting) return;
        entrada.target.setAttribute('data-visible', 'true');
        observador.unobserve(entrada.target);
      });
    }, { threshold: 0.08, rootMargin: '0px 0px -60px 0px' });
    animables.forEach(function (el) { observador.observe(el); });
    // Índice para escalonar los hijos
    $$('.reveal-stagger').forEach(function (grupo) {
      Array.prototype.forEach.call(grupo.children, function (hijo, i) {
        if (!hijo.style.getPropertyValue('--i')) hijo.style.setProperty('--i', i);
      });
    });
  }

  /* ── 10. Sección activa en el menú ──────────────────────────────────── */
  var enlaces = $$('.nav__link');
  var secciones = enlaces
    .map(function (a) { return document.getElementById(a.getAttribute('href').slice(1)); })
    .filter(Boolean);

  if (secciones.length && 'IntersectionObserver' in window) {
    var spy = new IntersectionObserver(function (entradas) {
      entradas.forEach(function (entrada) {
        if (!entrada.isIntersecting) return;
        enlaces.forEach(function (a) {
          a.setAttribute('aria-current', String(a.getAttribute('href') === '#' + entrada.target.id));
        });
      });
    }, { rootMargin: '-45% 0px -50% 0px' });
    secciones.forEach(function (s) { spy.observe(s); });
  }
})();
