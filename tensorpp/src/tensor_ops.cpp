// tensor_ops.cpp - Parte 2 (Dylanvoa14): sobrecarga de operadores.
//
// Todas las operaciones binarias entre tensores son elemento a elemento,
// dejan intactos a los operandos y devuelven un tensor nuevo con memoria
// propia. La multiplicacion matricial NO va aqui: es `matmul`, una funcion
// amiga de la Parte 3.
//
// Compatibilidad de dimensiones
// -----------------------------
// Dos formas son compatibles si, alineadas por la derecha, cada par de
// dimensiones o bien es igual, o bien una de las dos vale 1 (esa dimension
// se repite). Es la regla de difusion (broadcasting) de NumPy/PyTorch, y es
// lo que hace posible el paso 4 de la red neuronal: sumar un bias de 1x100
// a un tensor de 1000x100. Si no son compatibles se lanza ShapeError.
//
//     {2,3} y {2,3}  -> {2,3}      (formas iguales)
//     {1000,100} y {1,100} -> {1000,100}   (la fila del bias se repite)
//     {2,3} y {3}    -> {2,3}      (se alinea por la derecha)
//     {2,3} y {2}    -> ShapeError (3 y 2 no son compatibles)

#include "tensor.h"

#include <string>
#include <utility>
#include <vector>

namespace tensorpp {
namespace {

// Representacion en 3D de una forma de 1, 2 o 3 dimensiones: se rellena por
// la izquierda con unos. Asi todo el recorrido se escribe con tres bucles
// sin importar el numero real de dimensiones.
struct Vista3D {
  std::size_t dim[3];     // tamano de cada eje ya normalizado a 3D
  std::size_t stride[3];  // salto en el buffer; 0 en los ejes difundidos
};

std::string forma_a_texto(const std::vector<std::size_t>& shape) {
  std::string texto = "{";
  for (std::size_t i = 0; i < shape.size(); ++i) {
    if (i > 0) texto += ",";
    texto += std::to_string(shape[i]);
  }
  return texto + "}";
}

// Forma resultante de difundir `a` y `b`. Lanza ShapeError si son
// incompatibles.
std::vector<std::size_t> forma_difundida(const std::vector<std::size_t>& a,
                                         const std::vector<std::size_t>& b) {
  const std::size_t n = a.size() > b.size() ? a.size() : b.size();
  std::vector<std::size_t> salida(n);
  for (std::size_t i = 0; i < n; ++i) {
    // Se recorre desde la derecha: el eje i-esimo por la derecha de la
    // salida se corresponde con el i-esimo por la derecha de cada operando.
    const std::size_t da = i < a.size() ? a[a.size() - 1 - i] : 1;
    const std::size_t db = i < b.size() ? b[b.size() - 1 - i] : 1;
    if (da != db && da != 1 && db != 1) {
      throw ShapeError("dimensiones incompatibles: " + forma_a_texto(a) +
                       " y " + forma_a_texto(b));
    }
    salida[n - 1 - i] = da > db ? da : db;
  }
  return salida;
}

// Normaliza `shape` a 3 ejes y calcula los saltos necesarios para recorrer
// el buffer contiguo difundiendolo sobre `destino`. Un eje que se difunde
// (tamano 1 frente a un destino mayor, o eje inexistente) recibe salto 0,
// de modo que el mismo valor se relee en cada iteracion sin copiar nada.
Vista3D preparar(const std::vector<std::size_t>& shape,
                 const std::vector<std::size_t>& destino) {
  // Saltos contiguos (row-major) de la forma original.
  std::vector<std::size_t> saltos(shape.size(), 1);
  for (std::size_t i = shape.size(); i-- > 1;) {
    saltos[i - 1] = saltos[i] * shape[i];
  }

  Vista3D vista;
  for (std::size_t eje = 0; eje < 3; ++eje) {
    // Distancia del eje al extremo derecho: los ejes 3D sobrantes de la
    // izquierda no existen en la forma original.
    const std::size_t desde_derecha = 3 - 1 - eje;
    const bool existe_destino = desde_derecha < destino.size();
    const bool existe_origen = desde_derecha < shape.size();

    vista.dim[eje] =
        existe_destino ? destino[destino.size() - 1 - desde_derecha] : 1;
    if (!existe_origen) {
      vista.stride[eje] = 0;  // el operando no tiene este eje: se repite
      continue;
    }
    const std::size_t idx = shape.size() - 1 - desde_derecha;
    vista.stride[eje] = (shape[idx] == 1 && vista.dim[eje] != 1)
                            ? 0  // eje de tamano 1 difundido
                            : saltos[idx];
  }
  return vista;
}

enum class Operacion { Suma, Resta, Producto };

double aplicar(Operacion op, double x, double y) {
  switch (op) {
    case Operacion::Suma:
      return x + y;
    case Operacion::Resta:
      return x - y;
    case Operacion::Producto:
      return x * y;
  }
  return 0.0;  // inalcanzable; evita avisos del compilador
}

void exigir_no_vacio(const Tensor& t, const char* operacion) {
  if (t.empty()) {
    throw ShapeError(std::string("no se puede aplicar '") + operacion +
                     "' sobre un tensor vacio (posiblemente ya fue movido "
                     "por view/unsqueeze)");
  }
}

// Nucleo comun de +, - y *: recorre la forma difundida y escribe el
// resultado en un tensor nuevo. `a` y `b` no se modifican.
Tensor elemento_a_elemento(const Tensor& a, const Tensor& b, Operacion op,
                           const char* nombre) {
  exigir_no_vacio(a, nombre);
  exigir_no_vacio(b, nombre);

  const std::vector<std::size_t> forma = forma_difundida(a.shape(), b.shape());
  const Vista3D va = preparar(a.shape(), forma);
  const Vista3D vb = preparar(b.shape(), forma);

  Tensor resultado(forma);
  double* salida = resultado.data();
  const double* pa = a.data();
  const double* pb = b.data();

  std::size_t k = 0;
  for (std::size_t i0 = 0; i0 < va.dim[0]; ++i0) {
    const std::size_t oa0 = i0 * va.stride[0];
    const std::size_t ob0 = i0 * vb.stride[0];
    for (std::size_t i1 = 0; i1 < va.dim[1]; ++i1) {
      const std::size_t oa1 = oa0 + i1 * va.stride[1];
      const std::size_t ob1 = ob0 + i1 * vb.stride[1];
      for (std::size_t i2 = 0; i2 < va.dim[2]; ++i2) {
        salida[k++] = aplicar(op, pa[oa1 + i2 * va.stride[2]],
                              pb[ob1 + i2 * vb.stride[2]]);
      }
    }
  }
  return resultado;  // se devuelve por movimiento
}

}  // namespace

// --- Operadores binarios entre tensores -----------------------------------

Tensor Tensor::operator+(const Tensor& other) const {
  return elemento_a_elemento(*this, other, Operacion::Suma, "+");
}

Tensor Tensor::operator-(const Tensor& other) const {
  return elemento_a_elemento(*this, other, Operacion::Resta, "-");
}

// Producto de Hadamard (elemento a elemento). El producto matricial es
// `matmul`, declarado como funcion amiga.
Tensor Tensor::operator*(const Tensor& other) const {
  return elemento_a_elemento(*this, other, Operacion::Producto, "*");
}

// --- Operadores con escalar -----------------------------------------------

Tensor Tensor::operator*(double scalar) const {
  exigir_no_vacio(*this, "*");
  Tensor resultado(shape_);
  for (std::size_t i = 0; i < size_; ++i) {
    resultado.data_[i] = data_[i] * scalar;
  }
  return resultado;
}

Tensor operator*(double scalar, const Tensor& tensor) {
  return tensor * scalar;  // el producto por escalar es conmutativo
}

Tensor Tensor::operator-() const {
  return *this * -1.0;
}

// --- Operadores compuestos ------------------------------------------------
//
// Modifican este tensor in-place. La forma del resultado debe seguir siendo
// la propia, asi que el otro operando puede difundirse sobre este pero no al
// reves (por ejemplo `fila_1x100 += matriz_1000x100` es un error).

namespace {

Tensor& acumular(Tensor& destino, const Tensor& other, Operacion op,
                 const char* nombre) {
  Tensor calculado = elemento_a_elemento(destino, other, op, nombre);
  if (calculado.shape() != destino.shape()) {
    throw ShapeError(std::string("el operador '") + nombre +
                     "=' no puede cambiar la forma del tensor destino");
  }
  destino = std::move(calculado);
  return destino;
}

}  // namespace

Tensor& Tensor::operator+=(const Tensor& other) {
  return acumular(*this, other, Operacion::Suma, "+");
}

Tensor& Tensor::operator-=(const Tensor& other) {
  return acumular(*this, other, Operacion::Resta, "-");
}

Tensor& Tensor::operator*=(const Tensor& other) {
  return acumular(*this, other, Operacion::Producto, "*");
}

Tensor& Tensor::operator*=(double scalar) {
  exigir_no_vacio(*this, "*");
  for (std::size_t i = 0; i < size_; ++i) {
    data_[i] *= scalar;
  }
  return *this;
}

// --- Comparacion ----------------------------------------------------------

bool Tensor::operator==(const Tensor& other) const {
  if (shape_ != other.shape_) return false;
  for (std::size_t i = 0; i < size_; ++i) {
    if (data_[i] != other.data_[i]) return false;
  }
  return true;
}

bool Tensor::operator!=(const Tensor& other) const { return !(*this == other); }

}  // namespace tensorpp
