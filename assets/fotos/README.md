# Fotos

Dejar aquí las fotos con **exactamente estos nombres**. La web las recoge sola.

Mientras un archivo no exista, en su hueco aparece un marcador con el bigote y la
palabra "foto pendiente" — no se ve un icono roto, pero tampoco se puede vender así.

| Archivo | Dónde sale | Tamaño recomendado | Qué debería mostrar |
|---|---|---|---|
| `portada.jpg` | Fondo de la portada | 2400 × 1600 px | El local o un waffle, con espacio oscuro a la izquierda para el titular |
| `local-01.jpg` | Historia (vertical, grande) | 1200 × 1600 px | Plano general del salón |
| `local-02.jpg` | Historia (cuadrada) | 1200 × 1200 px | Detalle: mesas, plantas, decoración |
| `local-03.jpg` | Historia (cuadrada) | 1200 × 1200 px | Un waffle recién servido |
| `champipollo.jpg` | Especialidades | 1200 × 900 px | El Champipollo |
| `tejano.jpg` | Especialidades | 1200 × 900 px | El Tejano |
| `sandwich-gringo.jpg` | Especialidades | 1200 × 900 px | El Sandwich Gringo |
| `galeria-01.jpg` | Galería (ancha) | 1600 × 800 px | Salón principal |
| `galeria-02.jpg` … `galeria-06.jpg` | Galería (cuadradas) | 1200 × 1200 px | Platos, café, detalles, juegos de mesa |
| `galeria-07.jpg` | Galería (ancha) | 1600 × 800 px | Fachada desde la avenida |
| `galeria-08.jpg`, `galeria-09.jpg` | Galería (cuadradas) | 1200 × 1200 px | Bebidas, ambiente |

## Recomendaciones

- **Formato:** `.jpg` con calidad 80. Si se prefiere `.webp`, cambiar también la
  extensión en `index.html` (buscar `assets/fotos/`).
- **Peso:** por debajo de 300 KB cada una; la de portada puede llegar a 500 KB.
  Una web de restaurante que tarda en cargar pierde reservas.
- **Recorte:** la web recorta al centro (`object-fit: cover`). Dejar aire
  alrededor del motivo principal.
- **Orientación:** respetar el formato de la tabla. Una foto vertical en un hueco
  ancho se recorta mucho.

## Los textos alternativos

Cada foto lleva su `alt` escrito en `index.html`, pensado para lo que debería
mostrar según la tabla. Si se pone una foto distinta, **actualizar el `alt`**:
lo leen los lectores de pantalla y también Google.

## Derechos

Usar solo fotos propias del negocio o con permiso explícito. Las fotos publicadas
en Instagram por clientes son de quien las tomó, no del local.
