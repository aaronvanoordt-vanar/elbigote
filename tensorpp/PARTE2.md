# Tensor++ — Parte 2 (Dylanvoa14)

Sobrecarga de operadores y modificación de dimensiones: secciones **5**, **6**
y **7** del enunciado.

| Sección del enunciado | Qué se implementó | Archivo |
|---|---|---|
| 5. Sobrecarga de operadores | `+`, `-`, `*` (Hadamard), `* escalar`, `+=`, `-=`, `*=`, `-` unario, `==`, `!=` | `src/tensor_ops.cpp` |
| 6.1 `view` | reinterpretar la forma sin copiar datos | `src/tensor_shape.cpp` |
| 6.2 `unsqueeze` | insertar un eje de tamaño 1 sin copiar datos | `src/tensor_shape.cpp` |
| 7. `concat` | unir tensores por un eje, con memoria nueva | `src/tensor_shape.cpp` |

Las declaraciones están en `include/tensor.h`. La Parte 2 no toca la gestión de
memoria (Parte 1) ni `dot`/`matmul`/red neuronal (Parte 3).

## Compilar y ejecutar

```bash
make test   # 67 pruebas de la Parte 2
make demo   # ejemplos del enunciado
make asan   # las mismas pruebas bajo AddressSanitizer + UBSan
```

Requiere `g++` con C++17. Compila sin avisos con `-Wall -Wextra -Wpedantic` y
pasa limpio bajo ASan/UBSan (sin fugas ni dobles liberaciones).

## Sección 5 — Operadores

```cpp
Tensor C = A + B;
Tensor D = A - B;
Tensor E = A * B;    // elemento a elemento (Hadamard)
Tensor F = A * 2.0;  // por escalar; también 2.0 * A
```

- Los operandos **no se modifican**: los operadores son `const` y devuelven un
  tensor nuevo con memoria propia.
- `A * B` es el producto **elemento a elemento**. El producto matricial es
  `matmul`, la función amiga de la sección 8 (Parte 3).
- Si las dimensiones no son compatibles se lanza `tensorpp::ShapeError`
  (derivada de `std::invalid_argument`).

### Compatibilidad de dimensiones

Dos formas son compatibles si, **alineadas por la derecha**, cada par de
dimensiones es igual o alguna de las dos vale 1 (esa dimensión se repite). Es
la regla de difusión de NumPy/PyTorch:

| Operación | Resultado |
|---|---|
| `{2,3}` con `{2,3}` | `{2,3}` |
| `{1000,100}` con `{1,100}` | `{1000,100}` — la fila del bias se repite |
| `{2,3}` con `{3}` | `{2,3}` — se alinea por la derecha |
| `{1,3}` con `{2,1}` | `{2,3}` — ambos se expanden |
| `{2,3}` con `{2,2}` | `ShapeError` |

Hacía falta soportar esto porque el **paso 4 de la red neuronal** (sección 9)
suma un bias de `1×100` a un tensor de `1000×100`. La difusión no copia el
operando pequeño: el eje difundido recibe salto 0 y su valor se relee en cada
iteración.

## Sección 6 — `view` y `unsqueeze`

```cpp
Tensor A = Tensor::arange(0, 12);
Tensor B = A.view({3, 4});      // B reutiliza el mismo buffer que tenía A

Tensor P = Tensor::arange(0, 3);
Tensor Q = P.unsqueeze(0);      // {1,3}
```

El enunciado pide que estos métodos **no copien los datos** y que «trasladen la
información al tensor resultante», dejando el original en un estado válido. Se
implementó exactamente así: el puntero pasa al tensor resultante y **el tensor
original queda válido pero nulo** (`empty() == true`), igual que el origen de un
constructor de movimiento. `B.data() == ` el buffer que tenía `A`, verificado en
las pruebas.

Consecuencia práctica: el ejemplo de la página 4 del enunciado aplica
`unsqueeze` dos veces sobre el mismo `A`, y eso es incompatible con «no copiar».
Para ese caso se añadió `clone()`, que pide la copia de forma explícita:

```cpp
Tensor B = A.clone().unsqueeze(0);   // {1,3}, A conserva sus datos
Tensor C = A.clone().unsqueeze(1);   // {3,1}
```

Validaciones (todas lanzan `ShapeError`):

- `view` con un número distinto de elementos.
- `view` o `unsqueeze` que dejarían más de 3 dimensiones.
- `unsqueeze` con una posición fuera de `[0, ndim()]` (`dim == ndim()` es válido
  y agrega el eje al final).
- `view` o `unsqueeze` sobre un tensor que ya cedió sus datos, con un mensaje que
  explica la causa.

## Sección 7 — `concat`

```cpp
Tensor A = Tensor::ones({2, 3});
Tensor B = Tensor::zeros({2, 3});
Tensor C = Tensor::concat({A, B}, 0);   // {4,3}
Tensor D = Tensor::concat({A, B}, 1);   // {2,6}
```

- Valida que todos los tensores tengan el mismo número de dimensiones y
  coincidan en todos los ejes salvo el de concatenación.
- Reserva memoria nueva y copia los datos de forma controlada; los tensores de
  entrada no se modifican.
- Devuelve el resultado por movimiento.

El buffer es contiguo *row-major*, así que se recorre como `[externo][eje][interno]`:
para cada índice del bloque externo se pegan, en orden, las porciones de cada
tensor. Esto hace que la concatenación por el eje 2 de un 3D intercale los
bloques correctamente, no solo los pegue al final.

## Pruebas

`tests/test_parte2.cpp` — 67 comprobaciones, sin dependencias externas:

- operadores con formas iguales y con difusión (2D y 3D), incluido que los
  operandos no se modifiquen y que el resultado use memoria nueva;
- todos los casos de dimensiones incompatibles;
- operadores compuestos y comparación;
- `view`/`unsqueeze`: forma resultante, **identidad del puntero** (prueba de que
  no hay copia), estado del original, encadenado, y todos los errores;
- `concat` por cada eje en 1D/2D/3D, con más de dos tensores y tamaños distintos
  en el eje concatenado, más todos los errores;
- un ciclo de 200 iteraciones que combina `view`, `concat` y operadores, pensado
  para correr bajo `make asan`.

## Notas de integración

- `src/tensor_core.cpp` es una base mínima de la **Parte 1** para que la Parte 2
  compile y se pueda probar por sí sola. Al integrar, reemplazarlo por la
  versión de la Parte 1: la Parte 2 solo depende de la interfaz de
  `include/tensor.h`.
- `dot` y `matmul` están declaradas como funciones amigas en el header y las
  implementa la **Parte 3** en `src/tensor_linalg.cpp`. Al agregar ese archivo
  hay que sumarlo a la variable `FUENTES` del `Makefile`.
- El almacenamiento es un único `double*` contiguo, una de las tres opciones que
  permite el enunciado, y la que hace posible que `view` y `unsqueeze` no copien.
