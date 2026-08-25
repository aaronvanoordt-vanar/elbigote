# Qué falta antes de publicar

La web ya lleva **las fotos reales, la carta real con precios y el logotipo oficial**.
Este documento recoge lo que queda pendiente, de dónde sale cada dato y cómo
mantener el contenido.

---

## 1. Bloqueantes

### 1.1 La URL real

`https://elbigote.pe/` es un **marcador de posición**. Si se publica así, Google
recibe una URL canónica que no es la de la web y la indexación se rompe.

```bash
grep -rl 'elbigote.pe' . --exclude-dir=.git --exclude-dir=assets \
  | xargs sed -i 's|https://elbigote.pe|https://LA-URL-REAL|g'
```

Afecta a `index.html` (canonical, `og:url`, `og:image`, `twitter:image` y los
`@id` de los dos bloques JSON-LD), `sitemap.xml`, `robots.txt` y
`tools/generar_carta.py` (variable `base`).

### 1.2 Los reels de Instagram

La sección **Reels** funciona: muestra una portada propia y carga el vídeo de
Instagram sólo cuando el visitante pulsa. Pero los tres identificadores que
trae **vienen de la maqueta anterior y no se han podido comprobar** — desde
aquí no hay acceso a Instagram.

Antes de publicar hay que abrir la web y confirmar que los tres reproducen. Si
alguno no carga, sustituirlo en `js/main.js`:

```js
var REELS = [
  { id: 'DE7rVejufGc', poster: 'assets/fotos/portada.jpg',
    es: 'Waffles con helado', en: 'Waffles with ice cream' },
  …
];
```

El `id` es lo que va después de `/reel/` en la URL:
`https://www.instagram.com/reel/`**`DE7rVejufGc`**`/`

El `poster` es la imagen de portada; puede ser cualquier foto de `assets/fotos/`.
Se pueden poner más de tres: la rejilla se adapta sola.

---

## 2. Datos a confirmar con el negocio

| Dato | En la web | Conflicto | Dónde se cambia |
|---|---|---|---|
| **Hora de cierre** | 21:15 | El Comercio dice 21:15; otros listados dicen 21:45 | `index.html` (horarios, portada, pie, JSON-LD) **y** `js/main.js` → `HORARIO` |
| **Número de la calle** | 1450 | El Comercio y TripAdvisor dicen 1450; Restaurant Guru dice 1460 | `index.html`, enlaces a Google Maps |
| **Código postal** | 15063 | TripAdvisor dice 15003 | `index.html` (dirección y JSON-LD) |
| **Año de apertura** | 2017 | Deducido: un artículo de ~2021 dice "hace cuatro años" | `index.html` (portada y tarjeta de reseñas) |
| **WiFi** | *no se menciona* | Ninguna fuente lo confirma. Si lo hay, añadir una píldora en "En el local" | `index.html` |
| **Email** | *no se menciona* | No se encontró ninguno verificable | Añadir en "Visítanos" si existe |

> El horario vive en **dos** sitios: el texto de `index.html` y la constante
> `HORARIO` de `js/main.js`, que calcula el "Abierto ahora". Cambiar los dos.
> ```js
> var HORARIO = { abre: 9 * 60, cierra: 21 * 60 + 15 };  // minutos desde medianoche
> ```

---

## 3. Mantener la carta

**163 platos con sus precios**, transcritos de las imágenes oficiales de la
carta y traducidos al inglés.

La fuente de verdad es `tools/carta_datos.py`. El HTML de la carta **se genera**;
no editarlo a mano en `index.html` (se sobrescribe).

```bash
# 1. editar tools/carta_datos.py
# 2. regenerar
python3 tools/generar_carta.py
```

Eso reescribe el bloque entre `<!-- CARTA:INICIO -->` y `<!-- CARTA:FIN -->`,
el JSON-LD de la carta, y vuelca `data/carta.json`.

### Formato de un plato

```python
("Champipollo", "Champipollo", "23",
 "Champiñones, crema de leche y especias.",
 "Chicken with mushrooms, cream and spices.", ["top"]),
```

`(nombre_es, nombre_en, precio, descripción_es, descripción_en, etiquetas)`

- **precio**: cadena tal cual se imprime — `"23"`, `"6.5 – 8.5"`, `"9 – 11"`.
  `None` deja el plato sin precio.
- **etiquetas**: `"top"` = el más pedido · `"veg"` = sin carne.
  La etiqueta *sin carne* sólo se usa donde aporta: en los dulces y las bebidas
  se quitó a propósito, porque marcarlo en todos era ruido.

### Añadir una sección

Copiar un `dict(...)` completo dentro de `CATEGORIAS`. El campo `cat` decide en
qué pestaña sale, y admite varias separadas por espacio:

`waffles` · `salado` · `desayuno` · `dulce` · `bebidas` · `cafe`

Las pestañas se definen en `index.html`, en el `<div class="tabs">`.

### Los dos idiomas

Cada texto se escribe **dos veces** en el HTML, marcado con `lang="es"` y
`lang="en"`. El CSS enseña uno y esconde el otro, así que el cambio es
instantáneo, ambos idiomas quedan en el HTML para Google, y sin JavaScript se
ve el español.

El resto de la web (menús, botones, preguntas frecuentes) sigue el mismo patrón
directamente en `index.html`:

```html
<span lang="es">Ver la carta</span><span lang="en">See the menu</span>
```

Los textos que genera el JavaScript (horario, contadores, reels) están en el
objeto `TEXTOS` de `js/main.js`.

---

## 4. Fotos

Las 16 fotos están en `assets/fotos/`, redimensionadas y optimizadas
(ninguna pasa de 300 KB). Vinieron del material del negocio.

Si se sustituye alguna, **mantener el nombre del archivo** y actualizar el
`alt` correspondiente en `index.html` si cambia lo que se ve. La lista completa
está en `assets/fotos/README.md`.

Cuatro fotos llevan el nombre del plato rotulado encima (son piezas de
Instagram). Se usan a propósito en galería y destacados, donde ese rótulo
funciona; para la portada y los reels se eligieron fotos sin texto.

---

## 5. Reseñas

Las tres citas son fragmentos reales de TripAdvisor y de Google Local Guides,
atribuidos a la plataforma y **sin nombres de persona inventados**.

Conviene sustituirlas por reseñas verbatim recientes del panel de Google
Business. Están en `index.html`, sección `#resenas`. Las valoraciones numéricas
se actualizan ahí **y** en el `aggregateRating` del JSON-LD.

---

## 6. El logotipo

El logotipo que se usa es **el oficial del negocio**, extraído de la carta
(`Desayuno.png`) y recortado con fondo transparente:

| Archivo | Uso |
|---|---|
| `logo-elbigote.png` | Taza azul. Cabecera cuando está fijada, fondos claros |
| `logo-elbigote-claro.png` | Taza crema con letras azules. Cabecera sobre la portada, pie, 404 |
| `bigote.png` | Sólo el bigote. Filigrana de portada, viñetas de la cinta |
| `favicon-32 · apple-touch-icon · icon-192 · icon-512` | Iconos, generados del mismo bigote |
| `og.jpg` | Imagen para redes sociales |

Se extrajo de una imagen de 640 px, así que **la resolución es limitada**: se ve
bien hasta unos 200 px de alto. Si el negocio tiene el logotipo original en
vector o en alta resolución, sustituir estos archivos mejora el resultado.
Los tamaños y proporciones deben mantenerse.

---

## 7. De dónde sale cada dato

| Dato en la web | Fuente |
|---|---|
| Los 163 platos, precios, descripciones y adicionales | Imágenes oficiales de la carta |
| Logotipo | Imagen oficial de la carta (`Desayuno.png`) |
| Paleta azul #004F9C sobre crema #FCF6E8 | Muestreada de las imágenes de la carta |
| Fotos del local, platos y clientes | Material del negocio |
| Alessia y Diego, fundadores | El Comercio · *Provecho* |
| Horario 9:00–21:15, lunes a domingo | El Comercio · *Provecho* |
| Av. Almte. Miguel Grau 1450, Barranco | El Comercio; TripAdvisor |
| WhatsApp 936 819 234 y delivery por Rappi | El Comercio · *Provecho* |
| 4,6 en Google con 1.661 reseñas | Wanderlog / Google Local Guides |
| 4,5 en Restaurant Guru con 1.881 reseñas | Restaurant Guru |
| S/ 20–30 por persona | Restaurant Guru |
| Juegos de mesa; decoración vintage con placas de máquinas de bordar | Google Local Guides |
| Pet friendly | Directorio Mascotas365 |
| Zona norte de Barranco, junto al MAC | Google Local Guides |

**Descartado por no poder verificarse:** 27.000 seguidores en Instagram ·
"#16 de 263 cafeterías en Lima" (TripAdvisor lo sitúa en el puesto 318 de 2.952) ·
4,8 en Facebook con 631 reseñas · 1.296 reseñas en Google · horario 8:00–22:00 ·
el email `elbigotecafe@gmail.com` · "retratos de actores americanos con bigote" ·
reseñas firmadas por personas inventadas.

---

## 8. Repaso final antes de publicar

- [ ] URL real sustituida (punto 1.1)
- [ ] Los tres reels comprobados uno por uno (punto 1.2)
- [ ] Horario, dirección y código postal confirmados con el negocio
- [ ] Carta contrastada con la versión impresa vigente — los precios cambian
- [ ] Reseñas sustituidas por textos verbatim recientes
- [ ] Probado en un móvil real, no sólo en el navegador reducido
- [ ] Ficha de Google Business enlazando a la web
