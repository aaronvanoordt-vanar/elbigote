# El Bigote Coffee & Waffles — web comercial

Web de una sola página para **El Bigote Coffee & Waffles**, cafetería de waffles
en Av. Almte. Miguel Grau 1450, Barranco, Lima.

HTML, CSS y JavaScript planos. Sin build, sin dependencias, sin framework:
se publica tal cual en GitHub Pages, Netlify o cualquier hosting estático.

---

## Estructura

```
.
├── index.html              La web entera (contenido + datos estructurados)
├── 404.html                Página de error, autónoma
├── css/styles.css          Sistema de diseño: tokens, componentes, secciones
├── js/main.js              Idioma, horario en vivo, carta, reels, galería
├── tools/
│   ├── carta_datos.py      ← LA CARTA. Fuente de verdad, 163 platos ES/EN
│   └── generar_carta.py    Genera el HTML de la carta dentro de index.html
├── data/carta.json         La carta en JSON (se genera)
├── assets/
│   ├── brand/              Logotipo oficial, iconos, imagen para redes
│   └── fotos/              16 fotos del negocio, optimizadas
├── site.webmanifest · robots.txt · sitemap.xml
└── .nojekyll               Evita que GitHub Pages procese el repo con Jekyll
```

## Publicar en GitHub Pages

1. **Settings → Pages**
2. **Source:** *Deploy from a branch*
3. **Branch:** `main`, carpeta `/ (root)` → *Save*
4. En 1–2 minutos estará en `https://<usuario>.github.io/<repo>/`

Para dominio propio: añade un archivo `CNAME` en la raíz con el dominio y
apunta el DNS a GitHub Pages.

> **Antes de publicar** hay que revisar `CONTENIDO.md`: falta sustituir la URL
> de ejemplo y comprobar que los tres reels de Instagram cargan.

## Ver en local

```bash
python3 -m http.server 8000
# abre http://localhost:8000
```

## Editar la carta

La carta **no se edita en `index.html`** — se genera. Edita
`tools/carta_datos.py` y ejecuta:

```bash
python3 tools/generar_carta.py
```

Detalle del formato en `CONTENIDO.md`, sección 3.

---

## Qué trae

**Contenido**
- Portada, historia, carta completa, destacados, reels, galería, reseñas,
  cómo llegar y preguntas frecuentes.

**Bilingüe, español e inglés**
- Todo el sitio, no sólo la carta. Cada texto está dos veces en el HTML
  (`lang="es"` / `lang="en"`), así que el cambio es instantáneo, ambos idiomas
  quedan indexables y sin JavaScript se ve el español.
- Detecta el idioma del navegador la primera vez y recuerda la elección.

**Carta de verdad**
- 163 platos con precios reales, agrupados en 15 secciones.
- Pestañas por categoría y buscador que ignora tildes: `champinon`
  encuentra *Champiñones*, y busca también en inglés.
- En móvil las secciones se pliegan — con todo abierto serían 34.000 px de
  scroll; plegadas son 18.000.

**Funciona de verdad**
- Indicador **Abierto / Cerrado en vivo** calculado en hora de Lima
  (`America/Lima`), con aviso de "cerramos en X min" y el día de hoy resaltado.
- **Reels de Instagram** con portada propia: el iframe se carga sólo al pulsar,
  así la web no arrastra tres reproductores en cada visita.
- Galería con visor: teclado (`←` `→` `Esc`), foco atrapado y devuelto al salir.
- Menú móvil a pantalla completa y barra de acciones fija
  (Carta · Pedir · Llamar · Llegar).

**Buscadores y redes**
- JSON-LD `CafeOrCoffeeShop` con dirección, horario y valoración, `FAQPage`,
  y un `Menu` completo con los 163 platos y sus precios.
- Open Graph y Twitter Card con imagen propia.
- `sitemap.xml`, `robots.txt` y `canonical`.

**Accesibilidad**
- Enlace para saltar al contenido, un solo `<h1>` sin saltos de nivel,
  foco visible, `alt` en todas las imágenes, estados ARIA reales
  (`aria-expanded`, `aria-selected`, `aria-pressed`) y respeto de
  `prefers-reduced-motion`.

**Rendimiento**
- Cero dependencias de JavaScript. Sólo `styles.css` y `main.js`.
- Iconos como sprite SVG en línea, sin peticiones extra.
- Fotos optimizadas (ninguna pasa de 300 KB), `loading="lazy"` salvo la portada.

---

## La marca

Logotipo oficial del negocio, extraído de la carta y recortado con fondo
transparente, en dos versiones: azul para fondos claros y crema con letras
azules para fondos oscuros. El bigote se reutiliza como filigrana, viñeta e
iconos.

Paleta tomada directamente de la carta impresa: **azul `#004F9C`** sobre
**crema `#FCF6E8`**, con un dorado `#E2A33F` como acento cálido para las
llamadas a la acción — recoge el color de los waffles.

Tipografías: **Bricolage Grotesque** (titulares) y **Outfit** (texto), de
Google Fonts, con alternativas del sistema definidas.
