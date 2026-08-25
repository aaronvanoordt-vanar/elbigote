# El Bigote Coffee & Waffles — web comercial

Web de una sola página para **El Bigote Coffee & Waffles**, cafetería de waffles en
Av. Almte. Miguel Grau 1450, Barranco, Lima.

HTML, CSS y JavaScript planos. Sin build, sin dependencias, sin framework:
se publica tal cual en GitHub Pages, Netlify o cualquier hosting estático.

---

## Estructura

```
.
├── index.html              La web entera (contenido + datos estructurados)
├── 404.html                Página de error, autónoma
├── css/styles.css          Sistema de diseño: tokens, componentes, secciones
├── js/main.js              Horario en vivo, filtros de carta, galería, menú móvil
├── assets/
│   ├── brand/              Marca: bigote SVG, favicon, imagen para redes
│   ├── fotos/              ← AQUÍ VAN LAS FOTOS (ver assets/fotos/README.md)
│   └── carta/              Imágenes de la carta, si se quieren adjuntar
├── site.webmanifest        Instalable como app en móvil
├── robots.txt · sitemap.xml
└── .nojekyll               Evita que GitHub Pages procese el repo con Jekyll
```

## Publicar en GitHub Pages

1. **Settings → Pages**
2. **Source:** *Deploy from a branch*
3. **Branch:** `main`, carpeta `/ (root)` → *Save*
4. En 1–2 minutos estará en `https://<usuario>.github.io/<repo>/`

Para dominio propio: añade un archivo `CNAME` en la raíz con el dominio
(por ejemplo `elbigote.pe`) y apunta el DNS a GitHub Pages.

> **Antes de publicar** hay que revisar `CONTENIDO.md`. Faltan las fotos y hay
> tres datos que conviene confirmar con el negocio.

## Ver en local

```bash
python3 -m http.server 8000
# abre http://localhost:8000
```

Abrir `index.html` con doble clic también funciona, pero el mapa y las tipografías
se comportan mejor servidos por HTTP.

---

## Qué trae

**Contenido**
- Portada, historia, carta filtrable, especialidades, galería, reseñas, cómo llegar y preguntas frecuentes.

**Funciona de verdad**
- Indicador **Abierto / Cerrado en vivo** calculado en hora de Lima (`America/Lima`),
  con aviso de "cerramos en X min" y el día de hoy resaltado en la tabla de horarios.
- Carta con **pestañas de categoría y buscador** que ignora tildes
  (`champinon` encuentra *Champiñón*).
- Galería con visor: teclado (`←` `→` `Esc`), foco atrapado dentro y devuelto al salir.
- Menú móvil a pantalla completa y **barra de acciones fija** (Carta · Pedir · Llamar · Llegar)
  que aparece al pasar la portada.

**Buscadores y redes**
- JSON-LD `CafeOrCoffeeShop` con dirección, horario, valoración, carta y redes,
  más `FAQPage` para que las preguntas puedan salir en Google.
- Open Graph y Twitter Card con imagen propia (`assets/brand/og.png`).
- `sitemap.xml`, `robots.txt` y `canonical`.

**Accesibilidad**
- Enlace para saltar al contenido, un solo `<h1>`, foco visible en todo,
  `alt` en todas las imágenes, `aria-expanded` / `aria-selected` reales
  y respeto de `prefers-reduced-motion`.

**Rendimiento**
- Cero dependencias: solo `styles.css` (~28 KB) y `main.js` (~11 KB).
- Iconos como sprite SVG en línea, sin peticiones extra.
- Fotos con `loading="lazy"`; la de portada con `fetchpriority="high"`.
- Si una foto falta, aparece un marcador con el bigote en vez de un icono roto.

## Sobre la marca

El logotipo es un **bigote dibujado en SVG** (`assets/brand/bigote.svg`) que se
reutiliza como marca de cabecera, filigrana de la portada, viñeta de la cinta,
marcador de foto pendiente y favicon. Es un marcador de posición digno:
si el negocio tiene su logotipo oficial, ver `CONTENIDO.md` para cambiarlo.

Paleta: tostados (`#17100C` → `#E0A85C`) con un verde pistacho (`#7C9A63`)
como acento secundario. Tipografías: **Fraunces** (títulos) y **Outfit** (texto),
ambas de Google Fonts, con alternativas del sistema definidas.
