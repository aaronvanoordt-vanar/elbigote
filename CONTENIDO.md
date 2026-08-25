# Qué falta antes de publicar

Esta web se construyó **solo con datos verificables en fuentes públicas**. Nada está
inventado: lo que no se pudo confirmar, no se afirma. Este documento dice exactamente
qué falta, de dónde sale cada dato y cómo completarlo.

---

## 1. Bloqueantes — no publicar sin esto

### 1.1 La URL real

`https://elbigote.pe/` es un **marcador de posición**. Si se publica así, Google
recibirá una URL canónica que no es la de la web y la indexación se romperá.

Sustituir en estos sitios:

| Archivo | Dónde |
|---|---|
| `index.html` | `<link rel="canonical">`, `og:url`, `og:image`, `twitter:image`, y los `@id`/`url` del JSON-LD del final |
| `sitemap.xml` | `<loc>` |
| `robots.txt` | línea `Sitemap:` |

Buscar y reemplazar de una vez:

```bash
grep -rl 'elbigote.pe' . --exclude-dir=.git \
  | xargs sed -i 's|https://elbigote.pe|https://LA-URL-REAL|g'
```

### 1.2 Las fotos

La web funciona sin fotos — muestra un marcador con el bigote — pero **no se puede
vender así**. Los archivos que espera están listados en `assets/fotos/README.md`.

---

## 2. Datos a confirmar con el negocio

Fuentes públicas se contradicen en estos puntos. Elegimos la fuente más fiable
en cada caso, pero conviene confirmarlo por WhatsApp antes de publicar.

| Dato | En la web | Conflicto | Dónde se cambia |
|---|---|---|---|
| **Hora de cierre** | 21:15 | El Comercio dice 21:15; otros listados dicen 21:45 | `index.html` (tabla de horarios, portada, pie, JSON-LD) **y** `js/main.js` → `HORARIO` |
| **Número de la calle** | 1450 | El Comercio y TripAdvisor dicen 1450; Restaurant Guru dice 1460 | `index.html`, todos los enlaces a Google Maps |
| **Código postal** | 15063 | TripAdvisor dice 15003 | `index.html` (dirección y JSON-LD) |
| **Año de apertura** | 2017 | Deducido: un artículo de ~2021 dice "hace cuatro años" | `index.html` (kicker de portada y tarjeta de reseñas) |
| **WiFi** | *no se menciona* | Ninguna fuente lo confirma. Si lo hay, añadir la píldora en "En el local" (el icono `#i-wifi` ya existe) | `index.html` |
| **Email de contacto** | *no se menciona* | No se encontró ninguno verificable | Añadir en la sección "Visítanos" si existe |

> El horario vive en **dos** sitios: el texto de `index.html` y la constante
> `HORARIO` de `js/main.js`, que calcula el "Abierto ahora". Cambiar los dos.
> ```js
> var HORARIO = { abre: 9 * 60, cierra: 21 * 60 + 15 };  // minutos desde medianoche
> ```

---

## 3. Completar la carta

Los nombres que aparecen son los que se pudieron verificar. **No hay precios**
porque no se encontró ninguna lista de precios pública, y poner cifras inventadas
en la web de un restaurante es peor que no poner ninguna.

Mientras no haya precios, cada plato se muestra solo con nombre y descripción, y
una nota al pie remite al WhatsApp. En cuanto se tengan, la web ya está preparada.

### Añadir un plato

Duplicar este bloque dentro del `<ul class="menu-list">` de la sección que toque:

```html
<li class="menu-item" data-name="nombre en minúsculas y sin tildes">
  <span class="menu-item__name">Nombre del plato</span>
  <span class="menu-item__price">S/ 24</span>
  <span class="menu-item__desc">Descripción corta de los ingredientes.</span>
</li>
```

- `data-name` alimenta el buscador. Escribirlo **sin tildes**; el buscador ya
  normaliza lo que teclea el visitante, así que `champinon` encuentra *Champiñón*.
- Para que el precio se vea, **quitar** `data-empty="true"` del `<span>` del precio.
  Con ese atributo el precio queda oculto a propósito.
- Etiquetas opcionales dentro del nombre:
  ```html
  <span class="menu-item__flag" data-flag="top">El más pedido</span>
  <span class="menu-item__flag" data-flag="veg">Vegetariano</span>
  ```
- El contador de platos de cada sección es automático.

### Añadir una sección entera

Copiar un `<section class="menu-group">` completo. El atributo `data-cat` decide
en qué pestaña aparece; admite varias palabras separadas por espacio:

`waffles` · `salado` · `desayuno` · `bebidas`

Las pestañas se definen en el `<div class="tabs">` unas líneas más arriba.

### Si se prefiere usar las imágenes de la carta

Si el negocio tiene la carta como imágenes, se pueden dejar en `assets/carta/` y
enlazarlas desde la nota al pie de la sección. Aun así conviene mantener el texto:
Google no lee los precios dentro de un PNG, y en móvil obliga a hacer zoom.

---

## 4. Reseñas

Las tres citas son fragmentos reales recogidos de TripAdvisor y de Google Local
Guides, atribuidos a la plataforma y **sin nombres de persona inventados**.

Antes de publicar conviene sustituirlas por reseñas verbatim recientes copiadas
del panel de Google Business. Están en `index.html`, sección `#resenas`:

```html
<li class="card review">
  <p class="review__stars" aria-label="5 de 5 estrellas">★★★★★</p>
  <p class="review__quote">“Texto literal de la reseña.”</p>
  <p class="review__meta">Nombre · Google</p>
</li>
```

Las valoraciones numéricas (4,6 en Google con 1.661 reseñas) se actualizan en la
misma sección **y** en el `aggregateRating` del JSON-LD del final del archivo.
Conviene revisarlas cada pocos meses: si el número que declara el JSON-LD se aleja
del real, Google puede dejar de mostrar las estrellas en resultados.

---

## 5. El logotipo

La marca actual es un **bigote dibujado en SVG**, hecho para esta web. Es un
marcador de posición digno, no el logotipo oficial del negocio.

Si existe el logotipo oficial:

1. Dejarlo en `assets/brand/` (preferible SVG; si es PNG, con fondo transparente).
2. En `index.html` sustituir los dos bloques `<svg class="brand__mark">`
   (cabecera y pie) por `<img src="assets/brand/logo.svg" alt="El Bigote Coffee & Waffles">`.
3. Regenerar `assets/brand/favicon.svg`, `apple-touch-icon.png` y `og.png`.

El bigote se usa además como filigrana de la portada, viñeta de la cinta y
marcador de foto pendiente. Puede convivir con el logotipo oficial como
elemento gráfico secundario.

---

## 6. De dónde sale cada dato

| Dato en la web | Fuente |
|---|---|
| Alessia y Diego, fundadores; nacimiento del negocio | El Comercio · *Provecho* |
| Horario 9:00–21:15, lunes a domingo | El Comercio · *Provecho* |
| Av. Almte. Miguel Grau 1450, Barranco | El Comercio · *Provecho*; TripAdvisor |
| WhatsApp 936 819 234 y delivery por Rappi | El Comercio · *Provecho* |
| Champipollo (pollo saltado, champiñones, crema) y Tejano | El Comercio · *Provecho* |
| Sandwich Gringo | TripAdvisor (fotos del local) |
| Secciones de la carta (waffles, desayunos, sánguches, ensaladas, wraps, pastas, piqueos, cafés, bebidas, infusiones) | El Comercio; Restaurant Guru |
| 4,6 en Google con 1.661 reseñas | Wanderlog / Google Local Guides |
| 4,5 en Restaurant Guru con 1.881 reseñas | Restaurant Guru |
| S/ 20–30 por persona | Restaurant Guru |
| Juegos de mesa; decoración vintage con placas de máquinas de bordar | Google Local Guides |
| Pet friendly | Directorio Mascotas365 |
| "Cafetería de especialidad en waffles y café" | Web propia del negocio (WordPress) |
| Zona norte de Barranco, junto al MAC | Google Local Guides |
| Instagram, Facebook y TikTok | Perfiles oficiales |
| Smoothie de maracuyá y mango destacado | Reseña citada en la propia web |

**Datos del boceto anterior que se descartaron por no poder verificarse:**
27.000 seguidores en Instagram · "#16 de 263 cafeterías en Lima" (TripAdvisor lo
sitúa en el puesto 318 de 2.952 restaurantes) · 4,8 en Facebook con 631 reseñas ·
1.296 reseñas en Google (la cifra encontrada es 1.661) · horario 8:00–22:00 ·
el email `elbigotecafe@gmail.com` · "retratos de actores americanos con bigote"
en la decoración · reseñas firmadas por personas inventadas.

---

## 7. Repaso final antes de publicar

- [ ] URL real sustituida en los seis sitios del punto 1.1
- [ ] Fotos colocadas en `assets/fotos/` (ver su README)
- [ ] Horario, dirección y código postal confirmados con el negocio
- [ ] Precios de la carta añadidos (o decidido dejarlos fuera a propósito)
- [ ] Reseñas sustituidas por textos verbatim recientes
- [ ] Logotipo oficial colocado, si existe
- [ ] Probado en un móvil real, no solo en el navegador reducido
- [ ] Ficha de Google Business enlazando a la web
