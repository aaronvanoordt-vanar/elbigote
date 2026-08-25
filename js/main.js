/* ==========================================================================
   El Bigote Coffee & Waffles — comportamiento de la web
   Sin dependencias. Todo degrada con elegancia si algo falla.
   ========================================================================== */
(function () {
  'use strict';

  var $  = function (s, c) { return (c || document).querySelector(s); };
  var $$ = function (s, c) { return Array.prototype.slice.call((c || document).querySelectorAll(s)); };
  var reduceMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
  var raiz = document.documentElement;

  /* ── Configuración ──────────────────────────────────────────────────── */
  var HORARIO = { abre: 9 * 60, cierra: 21 * 60 + 15 };   // minutos desde medianoche
  var ZONA = 'America/Lima';

  /*  Reels destacados de Instagram.
      El identificador es lo que va después de /reel/ en la URL:
      https://www.instagram.com/reel/DE7rVejufGc/  →  'DE7rVejufGc'

      ⚠️  Estos tres vienen de la maqueta anterior y NO se han podido
      comprobar. Sustitúyelos por los reels que quieras destacar y revisa
      que cada uno cargue. Ver CONTENIDO.md. */
  var REELS = [
    { id: 'DE7rVejufGc', poster: 'assets/fotos/portada.jpg',
      es: 'Waffles con helado', en: 'Waffles with ice cream' },
    { id: 'DF3GPlURVrS', poster: 'assets/fotos/galeria-02-ensaladas.jpg',
      es: 'Almuerzos en Barranco', en: 'Lunch in Barranco' },
    { id: 'CpNoAPdA74i', poster: 'assets/fotos/galeria-05-mesa.jpg',
      es: 'El local por dentro', en: 'Inside the café' }
  ];

  var TEXTOS = {
    abierto:      { es: 'Abierto ahora · hasta las ',      en: 'Open now · until ' },
    cierraPronto: { es: 'Cerramos en ',                     en: 'Closing in ' },
    min:          { es: ' min',                             en: ' min' },
    abreHoy:      { es: 'Cerrado · abrimos hoy a las ',     en: 'Closed · opens today at ' },
    abreManana:   { es: 'Cerrado · abrimos mañana a las ',  en: 'Closed · opens tomorrow at ' },
    hoy:          { es: 'hoy',                              en: 'today' },
    plato:        { es: ' plato',                           en: ' dish' },
    platos:       { es: ' platos',                          en: ' dishes' },
    verReel:      { es: 'Ver el reel: ',                    en: 'Play reel: ' }
  };

  var idioma = 'es';
  function t(clave) { return TEXTOS[clave][idioma] || TEXTOS[clave].es; }

  /* ── 1. Idioma ──────────────────────────────────────────────────────── */
  function aplicarIdioma(nuevo, guardar) {
    idioma = (nuevo === 'en') ? 'en' : 'es';
    raiz.setAttribute('data-lang', idioma);
    raiz.setAttribute('lang', idioma === 'en' ? 'en' : 'es-PE');
    $$('[data-set-lang]').forEach(function (b) {
      b.setAttribute('aria-pressed', String(b.getAttribute('data-set-lang') === idioma));
    });
    var buscador = $('#menuSearch');
    if (buscador) {
      buscador.setAttribute('placeholder',
        buscador.getAttribute(idioma === 'en' ? 'data-ph-en' : 'data-ph-es') || '');
    }
    if (guardar) { try { localStorage.setItem('elbigote-idioma', idioma); } catch (e) {} }
    pintarEstado();
    actualizarContadores();
    pintarReels();
  }

  var guardado = null;
  try { guardado = localStorage.getItem('elbigote-idioma'); } catch (e) {}
  if (!guardado && (navigator.language || '').slice(0, 2).toLowerCase() !== 'es') guardado = 'en';

  $$('[data-set-lang]').forEach(function (b) {
    b.addEventListener('click', function () { aplicarIdioma(b.getAttribute('data-set-lang'), true); });
  });

  /* ── 2. Año ─────────────────────────────────────────────────────────── */
  $$('[data-year]').forEach(function (el) { el.textContent = String(new Date().getFullYear()); });

  /* ── 3. Abierto / cerrado en hora de Lima ───────────────────────────── */
  function horaLima() {
    try {
      var partes = new Intl.DateTimeFormat('en-US', {
        timeZone: ZONA, hour12: false, weekday: 'short', hour: '2-digit', minute: '2-digit'
      }).formatToParts(new Date());
      var v = {};
      partes.forEach(function (p) { v[p.type] = p.value; });
      var mapa = { Sun: 0, Mon: 1, Tue: 2, Wed: 3, Thu: 4, Fri: 5, Sat: 6 };
      var h = parseInt(v.hour, 10); if (h === 24) h = 0;
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
    var ahora = horaLima(), estado, texto;
    if (ahora.minutos >= HORARIO.abre && ahora.minutos < HORARIO.cierra) {
      var quedan = HORARIO.cierra - ahora.minutos;
      if (quedan <= 60) { estado = 'soon'; texto = t('cierraPronto') + quedan + t('min'); }
      else { estado = 'open'; texto = t('abierto') + hhmm(HORARIO.cierra); }
    } else {
      estado = 'closed';
      texto = (ahora.minutos < HORARIO.abre ? t('abreHoy') : t('abreManana')) + hhmm(HORARIO.abre);
    }
    $$('[data-status]').forEach(function (el) {
      el.setAttribute('data-state', estado);
      var s = $('[data-status-text]', el);
      if (s) s.textContent = texto;
    });
    $$('.hours__row').forEach(function (row) {
      row.setAttribute('data-today', String(Number(row.getAttribute('data-day')) === ahora.dia));
      row.style.setProperty('--hoy', '"' + t('hoy') + '"');
    });
  }
  setInterval(pintarEstado, 60000);

  /* ── 4. Cabecera y barra de acciones ────────────────────────────────── */
  var header = $('#siteHeader'), actionBar = $('#actionBar'), ticking = false;
  function alScroll() {
    var y = window.scrollY || window.pageYOffset;
    if (header) header.setAttribute('data-stuck', String(y > 40));
    if (actionBar) actionBar.setAttribute('data-show', String(y > window.innerHeight * 0.6));
  }
  window.addEventListener('scroll', function () {
    if (ticking) return;
    ticking = true;
    window.requestAnimationFrame(function () { alScroll(); ticking = false; });
  }, { passive: true });
  alScroll();

  /* ── 5. Panel móvil ─────────────────────────────────────────────────── */
  var burger = $('#burger'), drawer = $('#drawer');
  function abrirDrawer(abrir) {
    if (!burger || !drawer) return;
    drawer.setAttribute('data-open', String(abrir));
    burger.setAttribute('aria-expanded', String(abrir));
    document.body.style.overflow = abrir ? 'hidden' : '';
    if (abrir) {
      setTimeout(function () {   // el panel entra con transición de visibilidad
        var p = $('.drawer__link', drawer);
        if (p && drawer.getAttribute('data-open') === 'true') p.focus();
      }, 320);
    }
  }
  if (burger) burger.addEventListener('click', function () {
    abrirDrawer(drawer.getAttribute('data-open') !== 'true');
  });
  $$('.drawer__link, .drawer__foot a').forEach(function (a) {
    a.addEventListener('click', function () { abrirDrawer(false); });
  });

  /* ── 6. Carta: pestañas, buscador y contadores ──────────────────────── */
  var grupos = $$('.menu-group');
  var vacio = $('[data-menu-empty]');
  var buscador = $('#menuSearch');
  var filtroActivo = 'all';

  function normalizar(s) {
    return (s || '').toLowerCase().normalize('NFD').replace(/[\u0300-\u036f]/g, '');
  }

  function actualizarContadores() {
    grupos.forEach(function (g) {
      var n = $$('.menu-item', g).filter(function (i) { return !i.hidden; }).length;
      var c = $('[data-count]', g);
      if (c) c.textContent = n + (n === 1 ? t('plato') : t('platos'));
    });
  }

  function aplicarFiltros() {
    var q = normalizar(buscador ? buscador.value.trim() : '');
    var total = 0;
    grupos.forEach(function (grupo) {
      var cats = grupo.getAttribute('data-cat') || '';
      var pasaCat = filtroActivo === 'all' || cats.split(' ').indexOf(filtroActivo) !== -1;
      var visibles = 0;
      $$('.menu-item', grupo).forEach(function (item) {
        var texto = item.getAttribute('data-buscar') || '';
        var mostrar = pasaCat && (!q || texto.indexOf(q) !== -1);
        item.hidden = !mostrar;
        if (mostrar) visibles++;
      });
      grupo.hidden = visibles === 0;
      total += visibles;
    });
    if (vacio) vacio.hidden = total > 0;
    actualizarContadores();
    if (typeof estadoPlegado === 'function') estadoPlegado();
  }

  $$('.tab').forEach(function (tab) {
    tab.addEventListener('click', function () {
      $$('.tab').forEach(function (x) { x.setAttribute('aria-selected', 'false'); });
      tab.setAttribute('aria-selected', 'true');
      filtroActivo = tab.getAttribute('data-filter');
      aplicarFiltros();
    });
  });
  if (buscador) {
    var deb;
    buscador.addEventListener('input', function () {
      clearTimeout(deb); deb = setTimeout(aplicarFiltros, 120);
    });
  }


  /* ── 6b. Carta plegable en móvil ────────────────────────────────────
     Con 163 platos, dejarlo todo abierto en un teléfono son treinta mil
     píxeles de scroll. Se pliega por debajo de 860 px; al buscar o
     filtrar se abre solo para que los resultados se vean. */
  var esMovil = window.matchMedia('(max-width: 860px)');
  var plegadoManual = {};

  function claveGrupo(g) {
    var lista = $('.menu-list', g);
    return lista ? lista.id : (g.getAttribute('aria-labelledby') || '');
  }

  function estadoPlegado() {
    var buscando = !!(buscador && buscador.value.trim()) || filtroActivo !== 'all';
    grupos.forEach(function (g, i) {
      var abrir;
      if (!esMovil.matches || buscando) abrir = true;
      else if (claveGrupo(g) in plegadoManual) abrir = plegadoManual[claveGrupo(g)];
      else abrir = (i === 0);                       // la primera sección arranca abierta
      g.setAttribute('data-collapsed', String(!abrir));
      var b = $('.menu-group__toggle', g);
      if (b) b.setAttribute('aria-expanded', String(abrir));
    });
  }

  grupos.forEach(function (g) {
    var b = $('.menu-group__toggle', g);
    if (!b) return;
    b.addEventListener('click', function () {
      if (!esMovil.matches) return;               // en escritorio no se pliega
      var abierto = g.getAttribute('data-collapsed') !== 'true';
      plegadoManual[claveGrupo(g)] = !abierto;
      g.setAttribute('data-collapsed', String(abierto));
      b.setAttribute('aria-expanded', String(!abierto));
    });
  });
  if (esMovil.addEventListener) esMovil.addEventListener('change', function () { plegadoManual = {}; estadoPlegado(); });

  /* ── 7. Preguntas frecuentes ────────────────────────────────────────── */
  $$('.faq__item').forEach(function (item) {
    var b = $('.faq__q', item);
    if (!b) return;
    b.addEventListener('click', function () {
      var abierto = item.getAttribute('data-open') === 'true';
      item.setAttribute('data-open', String(!abierto));
      b.setAttribute('aria-expanded', String(!abierto));
    });
  });

  /* ── 8. Fotos que no cargan ─────────────────────────────────────────── */
  $$('.frame img, .hero__figura img').forEach(function (img) {
    function fallar() { img.hidden = true; }
    img.addEventListener('error', fallar);
    if (img.complete && img.naturalWidth === 0) fallar();
  });

  /* ── 9. Reels: el iframe de Instagram sólo se carga al pulsar ───────── */
  var rejilla = $('#reels-grid');

  function pintarReels() {
    if (!rejilla) return;
    // No repintar si ya hay un vídeo reproduciéndose
    if ($('iframe', rejilla)) return;
    rejilla.innerHTML = '';
    REELS.forEach(function (r) {
      var art = document.createElement('article');
      art.className = 'reel';
      var etiqueta = r[idioma] || r.es;
      art.innerHTML =
        '<button class="reel__facade" type="button" aria-label="' + t('verReel') + etiqueta + '">' +
          '<img src="' + r.poster + '" alt="" loading="lazy" decoding="async">' +
          '<span class="reel__play"><svg viewBox="0 0 24 24" fill="currentColor" aria-hidden="true">' +
            '<path d="M8 5.14v13.72a1 1 0 0 0 1.54.84l10.28-6.86a1 1 0 0 0 0-1.68L9.54 4.3A1 1 0 0 0 8 5.14Z"/>' +
          '</svg></span>' +
          '<span class="reel__cap">' + etiqueta + '<small>Instagram · @elbigotecoffeewaffles</small></span>' +
        '</button>';
      $('.reel__facade', art).addEventListener('click', function () {
        var f = document.createElement('iframe');
        f.src = 'https://www.instagram.com/reel/' + r.id + '/embed/';
        f.title = etiqueta;
        f.loading = 'lazy';
        f.setAttribute('allow', 'autoplay; encrypted-media; clipboard-write');
        f.setAttribute('allowfullscreen', '');
        art.innerHTML = '';
        art.appendChild(f);
      });
      rejilla.appendChild(art);
    });
  }

  /* ── 10. Visor de galería ───────────────────────────────────────────── */
  var lightbox = $('#lightbox'), lbImg = $('#lightboxImg'), lbCap = $('#lightboxCap');
  var botones = $$('[data-lightbox]');
  var fotos = botones.map(function (b) {
    var i = $('img', b);
    return { src: i ? i.getAttribute('src') : '', alt: i ? i.getAttribute('alt') : '' };
  });
  var indice = 0, origenFoco = null;

  function mostrarFoto(i) {
    if (!fotos.length) return;
    indice = (i + fotos.length) % fotos.length;
    lbImg.setAttribute('src', fotos[indice].src);
    lbImg.setAttribute('alt', fotos[indice].alt);
    lbCap.textContent = (indice + 1) + ' / ' + fotos.length + ' · ' + fotos[indice].alt;
  }
  function abrirVisor(i) {
    if (!lightbox) return;
    origenFoco = document.activeElement;
    mostrarFoto(i);
    lightbox.setAttribute('data-open', 'true');
    document.body.style.overflow = 'hidden';
    var c = $('[data-lb-close]', lightbox); if (c) c.focus();
  }
  function cerrarVisor() {
    if (!lightbox) return;
    lightbox.setAttribute('data-open', 'false');
    document.body.style.overflow = '';
    if (origenFoco && origenFoco.focus) origenFoco.focus();
  }
  botones.forEach(function (b, i) { b.addEventListener('click', function () { abrirVisor(i); }); });

  if (lightbox) {
    var cerrar = $('[data-lb-close]', lightbox);
    var prev = $('[data-lb-prev]', lightbox);
    var next = $('[data-lb-next]', lightbox);
    if (cerrar) cerrar.addEventListener('click', cerrarVisor);
    if (prev) prev.addEventListener('click', function () { mostrarFoto(indice - 1); });
    if (next) next.addEventListener('click', function () { mostrarFoto(indice + 1); });
    lightbox.addEventListener('click', function (e) { if (e.target === lightbox) cerrarVisor(); });
    lightbox.addEventListener('keydown', function (e) {
      if (e.key !== 'Tab') return;
      var f = $$('button', lightbox).filter(function (b) { return b.offsetParent !== null; });
      if (!f.length) return;
      var a = f[0], z = f[f.length - 1];
      if (e.shiftKey && document.activeElement === a) { e.preventDefault(); z.focus(); }
      else if (!e.shiftKey && document.activeElement === z) { e.preventDefault(); a.focus(); }
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

  /* ── 11. Aparición al hacer scroll ──────────────────────────────────── */
  var animables = $$('.reveal, .reveal-stagger');
  if (reduceMotion || !('IntersectionObserver' in window)) {
    animables.forEach(function (el) { el.setAttribute('data-visible', 'true'); });
  } else {
    var obs = new IntersectionObserver(function (es) {
      es.forEach(function (e) {
        if (!e.isIntersecting) return;
        e.target.setAttribute('data-visible', 'true');
        obs.unobserve(e.target);
      });
    }, { threshold: 0.06, rootMargin: '0px 0px -60px 0px' });
    animables.forEach(function (el) { obs.observe(el); });
    $$('.reveal-stagger').forEach(function (g) {
      Array.prototype.forEach.call(g.children, function (h, i) { h.style.setProperty('--i', i); });
    });
  }

  /* ── 12. Sección activa en el menú ──────────────────────────────────── */
  var enlaces = $$('.nav__link');
  var secciones = enlaces
    .map(function (a) { return document.getElementById(a.getAttribute('href').slice(1)); })
    .filter(Boolean);
  if (secciones.length && 'IntersectionObserver' in window) {
    var spy = new IntersectionObserver(function (es) {
      es.forEach(function (e) {
        if (!e.isIntersecting) return;
        enlaces.forEach(function (a) {
          a.setAttribute('aria-current', String(a.getAttribute('href') === '#' + e.target.id));
        });
      });
    }, { rootMargin: '-45% 0px -50% 0px' });
    secciones.forEach(function (s) { spy.observe(s); });
  }

  /* ── Arranque ───────────────────────────────────────────────────────── */
  aplicarIdioma(guardado || 'es', false);
  aplicarFiltros();
})();
