// test_parte2.cpp - Pruebas de la Parte 2 (Dylanvoa14):
//   * sobrecarga de operadores (+, -, *, * escalar y compuestos)
//   * view / unsqueeze / concat
//
// Arnes minimo sin dependencias externas: cada CHECK reporta la linea que
// fallo y al final se imprime el resumen. Devuelve 0 si todo pasa.

#include "tensor.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using tensorpp::ShapeError;
using tensorpp::Tensor;

namespace {

int pruebas_totales = 0;
int pruebas_fallidas = 0;

void reportar(bool ok, const std::string& descripcion, int linea) {
  ++pruebas_totales;
  if (ok) {
    std::cout << "  [ok]    " << descripcion << "\n";
  } else {
    ++pruebas_fallidas;
    std::cout << "  [FALLA] " << descripcion << "  (linea " << linea << ")\n";
  }
}

#define CHECK(cond, desc) reportar((cond), (desc), __LINE__)

// Comprueba que el tensor tenga la forma y los valores esperados.
bool coincide(const Tensor& t, const std::vector<std::size_t>& forma,
              const std::vector<double>& valores) {
  if (t.shape() != forma) return false;
  if (t.size() != valores.size()) return false;
  for (std::size_t i = 0; i < valores.size(); ++i) {
    if (std::fabs(t.at(i) - valores[i]) > 1e-12) return false;
  }
  return true;
}

// Ejecuta `accion` y devuelve true si lanzo ShapeError.
template <typename Accion>
bool lanza_shape_error(Accion accion) {
  try {
    accion();
  } catch (const ShapeError&) {
    return true;
  } catch (...) {
    return false;
  }
  return false;
}

void seccion(const std::string& titulo) {
  std::cout << "\n== " << titulo << " ==\n";
}

// -------------------------------------------------------------------------
// 1. Operadores
// -------------------------------------------------------------------------
void probar_operadores_basicos() {
  seccion("Operadores elemento a elemento");

  const Tensor a({2, 3}, {1, 2, 3, 4, 5, 6});
  const Tensor b({2, 3}, {10, 20, 30, 40, 50, 60});

  const Tensor suma = a + b;
  CHECK(coincide(suma, {2, 3}, {11, 22, 33, 44, 55, 66}), "A + B");

  const Tensor resta = b - a;
  CHECK(coincide(resta, {2, 3}, {9, 18, 27, 36, 45, 54}), "B - A");

  const Tensor producto = a * b;
  CHECK(coincide(producto, {2, 3}, {10, 40, 90, 160, 250, 360}),
        "A * B (Hadamard, no matricial)");

  // Los operandos no deben modificarse.
  CHECK(coincide(a, {2, 3}, {1, 2, 3, 4, 5, 6}), "A no se modifico");
  CHECK(coincide(b, {2, 3}, {10, 20, 30, 40, 50, 60}), "B no se modifico");

  // El resultado tiene memoria propia e independiente de los operandos.
  CHECK(suma.data() != a.data() && suma.data() != b.data(),
        "el resultado usa memoria nueva");
}

void probar_escalar() {
  seccion("Producto por escalar");

  const Tensor a({2, 2}, {1, 2, 3, 4});

  CHECK(coincide(a * 2.0, {2, 2}, {2, 4, 6, 8}), "A * 2.0");
  CHECK(coincide(2.0 * a, {2, 2}, {2, 4, 6, 8}), "2.0 * A (forma simetrica)");
  CHECK(coincide(a * -1.0, {2, 2}, {-1, -2, -3, -4}), "A * -1.0");
  CHECK(coincide(-a, {2, 2}, {-1, -2, -3, -4}), "-A (negacion unaria)");
  CHECK(coincide(a, {2, 2}, {1, 2, 3, 4}), "A no se modifico");
}

void probar_broadcast() {
  seccion("Compatibilidad de dimensiones (difusion)");

  // Caso de la red neuronal: sumar un bias 1xN a una matriz MxN.
  const Tensor lote({3, 2}, {1, 2, 3, 4, 5, 6});
  const Tensor bias({1, 2}, {10, 20});
  CHECK(coincide(lote + bias, {3, 2}, {11, 22, 13, 24, 15, 26}),
        "{3,2} + {1,2}: el bias se repite por filas");

  // Alineacion por la derecha con distinto numero de dimensiones.
  const Tensor fila({2}, {100, 200});
  CHECK(coincide(lote + fila, {3, 2}, {101, 202, 103, 204, 105, 206}),
        "{3,2} + {2}: se alinea por la derecha");

  // Difusion por columnas.
  const Tensor columna({3, 1}, {1, 2, 3});
  CHECK(coincide(lote * columna, {3, 2}, {1, 2, 6, 8, 15, 18}),
        "{3,2} * {3,1}: la columna se repite");

  // Difusion en 3D.
  const Tensor cubo({2, 2, 2}, {1, 2, 3, 4, 5, 6, 7, 8});
  const Tensor plano({1, 2, 2}, {10, 20, 30, 40});
  CHECK(coincide(cubo + plano, {2, 2, 2}, {11, 22, 33, 44, 15, 26, 37, 48}),
        "{2,2,2} + {1,2,2}");

  // Ambos operandos se expanden a la vez.
  const Tensor f({1, 3}, {1, 2, 3});
  const Tensor c({2, 1}, {10, 20});
  CHECK(coincide(f + c, {2, 3}, {11, 12, 13, 21, 22, 23}),
        "{1,3} + {2,1} -> {2,3}");
}

void probar_operadores_invalidos() {
  seccion("Operadores con dimensiones incompatibles");

  const Tensor a({2, 3}, {1, 2, 3, 4, 5, 6});
  const Tensor b({2, 2}, {1, 2, 3, 4});
  const Tensor c({2}, {1, 2});

  CHECK(lanza_shape_error([&] { return a + b; }),
        "{2,3} + {2,2} lanza ShapeError");
  CHECK(lanza_shape_error([&] { return a - b; }),
        "{2,3} - {2,2} lanza ShapeError");
  CHECK(lanza_shape_error([&] { return a * b; }),
        "{2,3} * {2,2} lanza ShapeError");
  CHECK(lanza_shape_error([&] { return a + c; }),
        "{2,3} + {2} lanza ShapeError (3 y 2 no son difundibles)");
}

void probar_operadores_compuestos() {
  seccion("Operadores compuestos y comparacion");

  Tensor a({2, 2}, {1, 2, 3, 4});
  a += Tensor({2, 2}, {1, 1, 1, 1});
  CHECK(coincide(a, {2, 2}, {2, 3, 4, 5}), "A += B");

  a -= Tensor({1, 2}, {1, 2});
  CHECK(coincide(a, {2, 2}, {1, 1, 3, 3}), "A -= bias difundido");

  a *= 3.0;
  CHECK(coincide(a, {2, 2}, {3, 3, 9, 9}), "A *= escalar");

  a *= Tensor({2, 2}, {1, 0, 1, 0});
  CHECK(coincide(a, {2, 2}, {3, 0, 9, 0}), "A *= B");

  // Un compuesto no puede cambiar la forma del destino.
  Tensor fila({1, 2}, {1, 2});
  CHECK(lanza_shape_error([&] { fila += Tensor({3, 2}, {1, 2, 3, 4, 5, 6}); }),
        "'+=' rechaza cambiar la forma del destino");

  CHECK(Tensor({2, 2}, {1, 2, 3, 4}) == Tensor({2, 2}, {1, 2, 3, 4}),
        "operator== con mismos valores");
  CHECK(Tensor({2, 2}, {1, 2, 3, 4}) != Tensor({4}, {1, 2, 3, 4}),
        "operator!= distingue la forma");
}

// -------------------------------------------------------------------------
// 2. view
// -------------------------------------------------------------------------
void probar_view() {
  seccion("view");

  Tensor a = Tensor::arange(0, 12);
  CHECK(a.shape() == std::vector<std::size_t>{12}, "arange(0,12) es 1D de 12");

  const double* buffer_original = a.data();
  Tensor b = a.view({3, 4});

  CHECK(b.shape() == (std::vector<std::size_t>{3, 4}), "view({3,4}) cambia la forma");
  CHECK(b.data() == buffer_original,
        "view NO copia: el resultado reutiliza el mismo buffer");
  CHECK(b.size() == 12, "view conserva el numero de elementos");
  CHECK(b(0, 0) == 0.0 && b(1, 0) == 4.0 && b(2, 3) == 11.0,
        "los datos se reinterpretan en orden row-major");

  // El tensor original queda valido pero nulo (como el origen de un move).
  CHECK(a.empty() && a.data() == nullptr,
        "el tensor original queda valido pero nulo");
  a = Tensor::ones({2, 2});  // sigue siendo asignable: esta valido
  CHECK(coincide(a, {2, 2}, {1, 1, 1, 1}), "el original se puede reutilizar");

  // Encadenar view sobre el resultado tambien funciona.
  Tensor c = b.view({2, 2, 3});
  CHECK(c.shape() == (std::vector<std::size_t>{2, 2, 3}) &&
            c.data() == buffer_original,
        "view encadenado a 3D sigue sin copiar");

  // Para conservar el original hay que pedir la copia explicitamente.
  Tensor d = Tensor::arange(0, 6);
  Tensor e = d.clone().view({2, 3});
  CHECK(!d.empty() && d.size() == 6, "clone().view() conserva el original");
  CHECK(e.data() != d.data(), "clone().view() usa memoria distinta");
}

void probar_view_invalido() {
  seccion("view invalido");

  CHECK(lanza_shape_error([] { Tensor t = Tensor::arange(0, 12); t.view({5, 3}); }),
        "view con otro numero de elementos lanza ShapeError");
  CHECK(lanza_shape_error([] { Tensor t = Tensor::arange(0, 16); t.view({2, 2, 2, 2}); }),
        "view con 4 dimensiones lanza ShapeError");
  CHECK(lanza_shape_error([] { Tensor t = Tensor::arange(0, 12); t.view({}); }),
        "view con forma vacia lanza ShapeError");
  CHECK(lanza_shape_error([] {
          Tensor t = Tensor::arange(0, 12);
          Tensor movido = t.view({3, 4});
          t.view({3, 4});  // t ya cedio sus datos
        }),
        "view sobre un tensor ya movido lanza ShapeError");
}

// -------------------------------------------------------------------------
// 3. unsqueeze
// -------------------------------------------------------------------------
void probar_unsqueeze() {
  seccion("unsqueeze");

  Tensor a = Tensor::arange(0, 3);
  const double* buffer_original = a.data();

  Tensor b = a.unsqueeze(0);
  CHECK(b.shape() == (std::vector<std::size_t>{1, 3}), "unsqueeze(0) -> {1,3}");
  CHECK(b.data() == buffer_original, "unsqueeze NO copia los datos");
  CHECK(a.empty(), "el original queda valido pero nulo");

  // El ejemplo del enunciado usa el mismo tensor dos veces; como unsqueeze
  // traslada los datos, la segunda vez se parte de una copia explicita.
  Tensor a2 = Tensor::arange(0, 3);
  Tensor c = a2.clone().unsqueeze(1);
  CHECK(c.shape() == (std::vector<std::size_t>{3, 1}), "unsqueeze(1) -> {3,1}");
  CHECK(!a2.empty(), "con clone() el original conserva sus datos");
  CHECK(coincide(c, {3, 1}, {0, 1, 2}), "unsqueeze conserva los valores");

  // Insertar al final es valido (dim == ndim()).
  Tensor d = Tensor({2, 2}, {1, 2, 3, 4});
  Tensor e = d.unsqueeze(2);
  CHECK(e.shape() == (std::vector<std::size_t>{2, 2, 1}),
        "unsqueeze(2) sobre {2,2} -> {2,2,1}");

  // Insertar en el medio.
  Tensor f = Tensor({2, 2}, {1, 2, 3, 4}).unsqueeze(1);
  CHECK(f.shape() == (std::vector<std::size_t>{2, 1, 2}),
        "unsqueeze(1) sobre {2,2} -> {2,1,2}");

  // Combinado con los operadores: preparar un vector como bias de fila.
  Tensor lote({2, 3}, {1, 2, 3, 4, 5, 6});
  Tensor bias = Tensor({3}, {10, 20, 30}).unsqueeze(0);  // {1,3}
  CHECK(coincide(lote + bias, {2, 3}, {11, 22, 33, 14, 25, 36}),
        "unsqueeze prepara un bias para sumarlo con difusion");
}

void probar_unsqueeze_invalido() {
  seccion("unsqueeze invalido");

  CHECK(lanza_shape_error([] {
          Tensor t({2, 2, 2}, {1, 2, 3, 4, 5, 6, 7, 8});
          t.unsqueeze(0);
        }),
        "unsqueeze sobre un tensor 3D lanza ShapeError");
  CHECK(lanza_shape_error([] {
          Tensor t = Tensor::arange(0, 3);
          t.unsqueeze(5);
        }),
        "unsqueeze con posicion fuera de rango lanza ShapeError");
  CHECK(lanza_shape_error([] {
          Tensor t = Tensor::arange(0, 3);
          Tensor movido = t.unsqueeze(0);
          t.unsqueeze(0);
        }),
        "unsqueeze sobre un tensor ya movido lanza ShapeError");
}

// -------------------------------------------------------------------------
// 4. concat
// -------------------------------------------------------------------------
void probar_concat() {
  seccion("concat");

  const Tensor a = Tensor::ones({2, 3});
  const Tensor b = Tensor::zeros({2, 3});

  const Tensor c = Tensor::concat({a, b}, 0);
  CHECK(coincide(c, {4, 3}, {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}),
        "concat({A,B}, 0) apila por filas -> {4,3}");

  const Tensor d = Tensor::concat({a, b}, 1);
  CHECK(coincide(d, {2, 6}, {1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0}),
        "concat({A,B}, 1) apila por columnas -> {2,6}");

  // Los tensores de entrada no se modifican y el resultado es independiente.
  CHECK(coincide(a, {2, 3}, {1, 1, 1, 1, 1, 1}), "A no se modifico");
  CHECK(c.data() != a.data() && c.data() != b.data(),
        "concat reserva memoria nueva");

  // Ejes de distinto tamano en la dimension concatenada.
  const Tensor p({1, 2}, {1, 2});
  const Tensor q({3, 2}, {3, 4, 5, 6, 7, 8});
  CHECK(coincide(Tensor::concat({p, q}, 0), {4, 2}, {1, 2, 3, 4, 5, 6, 7, 8}),
        "concat admite tamanos distintos en el eje concatenado");

  // Mas de dos tensores.
  const Tensor r = Tensor::concat(
      {Tensor({1}, {1}), Tensor({2}, {2, 3}), Tensor({1}, {4})}, 0);
  CHECK(coincide(r, {4}, {1, 2, 3, 4}), "concat de tres tensores 1D");

  // 3D por el ultimo eje: obliga a intercalar bloques, no solo a pegar.
  const Tensor cubo1({2, 2, 1}, {1, 2, 3, 4});
  const Tensor cubo2({2, 2, 2}, {10, 20, 30, 40, 50, 60, 70, 80});
  CHECK(coincide(Tensor::concat({cubo1, cubo2}, 2), {2, 2, 3},
                 {1, 10, 20, 2, 30, 40, 3, 50, 60, 4, 70, 80}),
        "concat 3D por el eje 2 intercala correctamente");

  // 3D por el eje del medio.
  CHECK(coincide(Tensor::concat({cubo2, cubo2}, 1), {2, 4, 2},
                 {10, 20, 30, 40, 10, 20, 30, 40, 50, 60, 70, 80, 50, 60, 70,
                  80}),
        "concat 3D por el eje 1");

  // Un solo tensor: copia con memoria propia.
  const Tensor solo = Tensor::concat({a}, 0);
  CHECK(coincide(solo, {2, 3}, {1, 1, 1, 1, 1, 1}) && solo.data() != a.data(),
        "concat de un solo tensor devuelve una copia independiente");
}

void probar_concat_invalido() {
  seccion("concat invalido");

  const Tensor a = Tensor::ones({2, 3});
  const Tensor b = Tensor::ones({2, 4});
  const Tensor c = Tensor::ones({2, 2, 2});

  CHECK(lanza_shape_error([] { return Tensor::concat({}, 0); }),
        "concat sin tensores lanza ShapeError");
  CHECK(lanza_shape_error([&] { return Tensor::concat({a, b}, 0); }),
        "concat con eje no concatenado distinto lanza ShapeError");
  CHECK(lanza_shape_error([&] { return Tensor::concat({a, c}, 0); }),
        "concat con distinto numero de dimensiones lanza ShapeError");
  CHECK(lanza_shape_error([&] { return Tensor::concat({a, a}, 2); }),
        "concat sobre un eje inexistente lanza ShapeError");
}

// -------------------------------------------------------------------------
// 5. Sin fugas: uso intensivo para ejecutar bajo valgrind / sanitizers
// -------------------------------------------------------------------------
void probar_ciclo_intensivo() {
  seccion("Uso intensivo (para valgrind / -fsanitize=address)");

  for (int i = 0; i < 200; ++i) {
    Tensor x = Tensor::arange(0, 24);
    Tensor y = x.view({2, 3, 4});
    Tensor z = y + Tensor::ones({1, 3, 4});
    Tensor w = Tensor::concat({z, z}, 0);
    Tensor v = (w * 2.0) - w;
    v = v * Tensor::ones({4, 3, 4});
    if (v.size() != 48) {
      CHECK(false, "el ciclo intensivo produjo un tamano inesperado");
      return;
    }
  }
  CHECK(true, "200 ciclos de view/concat/operadores sin errores");
}

}  // namespace

int main() {
  std::cout << "Pruebas de la Parte 2 (operadores, view, unsqueeze, concat)\n";

  probar_operadores_basicos();
  probar_escalar();
  probar_broadcast();
  probar_operadores_invalidos();
  probar_operadores_compuestos();
  probar_view();
  probar_view_invalido();
  probar_unsqueeze();
  probar_unsqueeze_invalido();
  probar_concat();
  probar_concat_invalido();
  probar_ciclo_intensivo();

  std::cout << "\n---------------------------------------------\n";
  std::cout << "Total: " << pruebas_totales << "  |  Fallidas: "
            << pruebas_fallidas << "\n";
  return pruebas_fallidas == 0 ? 0 : 1;
}
