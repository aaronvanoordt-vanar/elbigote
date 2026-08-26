// tensor_shape.cpp - Parte 2 (Dylanvoa14): view, unsqueeze y concat.
//
// `view` y `unsqueeze` reinterpretan la forma logica de un tensor SIN tocar
// los datos: el buffer se traslada (move) al tensor resultante y el tensor
// original queda en un estado valido pero nulo, exactamente como el origen
// de un constructor de movimiento. No hay ninguna copia de memoria.
//
//     Tensor A = Tensor::arange(0, 12);
//     Tensor B = A.view({3, 4});   // B posee los datos; A queda vacio
//
// Si se necesita conservar el tensor original hay que pedir la copia de
// forma explicita con `clone()`:
//
//     Tensor B = A.clone().view({3, 4});   // A conserva sus datos
//
// `concat` si reserva memoria nueva, copia los datos de los tensores de
// entrada (que no se modifican) y devuelve el resultado por movimiento.

#include "tensor.h"

#include <cstring>
#include <string>
#include <vector>

namespace tensorpp {
namespace {

std::string forma_a_texto(const std::vector<std::size_t>& shape) {
  std::string texto = "{";
  for (std::size_t i = 0; i < shape.size(); ++i) {
    if (i > 0) texto += ",";
    texto += std::to_string(shape[i]);
  }
  return texto + "}";
}

}  // namespace

// --- view -----------------------------------------------------------------

Tensor Tensor::view(const std::vector<std::size_t>& shape) {
  // num_elements valida de paso que la forma sea de 1 a 3 dimensiones y que
  // ninguna dimension sea cero.
  const std::size_t total = num_elements(shape);

  if (empty()) {
    throw ShapeError(
        "view sobre un tensor vacio: sus datos ya fueron trasladados por otra "
        "llamada a view/unsqueeze o por un movimiento");
  }
  if (total != size_) {
    throw ShapeError("view: la forma " + forma_a_texto(shape) + " requiere " +
                     std::to_string(total) + " elementos, pero el tensor " +
                     forma_a_texto(shape_) + " tiene " +
                     std::to_string(size_));
  }

  // Traslado de propiedad: el puntero pasa tal cual al resultado, solo
  // cambia la forma logica. Cero copias de datos.
  Tensor resultado;
  resultado.data_ = data_;
  resultado.shape_ = shape;
  resultado.size_ = size_;

  // El tensor original queda valido pero nulo (no libera nada: ya no es suyo).
  data_ = nullptr;
  shape_.clear();
  size_ = 0;

  return resultado;
}

// --- unsqueeze ------------------------------------------------------------

Tensor Tensor::unsqueeze(std::size_t dim) {
  if (empty()) {
    throw ShapeError(
        "unsqueeze sobre un tensor vacio: sus datos ya fueron trasladados por "
        "otra llamada a view/unsqueeze o por un movimiento");
  }
  if (ndim() + 1 > kMaxDims) {
    throw ShapeError("unsqueeze: el tensor " + forma_a_texto(shape_) +
                     " ya tiene 3 dimensiones y no admite una mas");
  }
  // `dim == ndim()` es valido: agrega el eje al final ({3} -> {3,1}).
  if (dim > ndim()) {
    throw ShapeError("unsqueeze: la posicion " + std::to_string(dim) +
                     " esta fuera de rango para un tensor de " +
                     std::to_string(ndim()) + " dimension(es)");
  }

  std::vector<std::size_t> nueva_forma = shape_;
  nueva_forma.insert(nueva_forma.begin() + static_cast<long>(dim), 1);

  // Agregar un eje de tamano 1 no altera el orden ni la cantidad de datos,
  // asi que se reutiliza `view` y con ella el mismo traslado sin copias.
  return view(nueva_forma);
}

// --- concat ---------------------------------------------------------------

Tensor Tensor::concat(const std::vector<Tensor>& tensors, std::size_t dim) {
  if (tensors.empty()) {
    throw ShapeError("concat necesita al menos un tensor");
  }

  const std::vector<std::size_t>& referencia = tensors.front().shape_;
  const std::size_t n_dims = referencia.size();
  if (dim >= n_dims) {
    throw ShapeError("concat: el eje " + std::to_string(dim) +
                     " no existe en un tensor de " + std::to_string(n_dims) +
                     " dimension(es)");
  }

  // Validacion: mismo numero de dimensiones y mismos tamanos en todos los
  // ejes salvo en el eje de concatenacion, que es el que se acumula.
  std::size_t total_eje = 0;
  for (std::size_t t = 0; t < tensors.size(); ++t) {
    const Tensor& actual = tensors[t];
    if (actual.empty()) {
      throw ShapeError("concat: el tensor en la posicion " +
                       std::to_string(t) + " esta vacio");
    }
    if (actual.shape_.size() != n_dims) {
      throw ShapeError("concat: el tensor en la posicion " +
                       std::to_string(t) + " tiene " +
                       std::to_string(actual.shape_.size()) +
                       " dimension(es) y se esperaban " +
                       std::to_string(n_dims));
    }
    for (std::size_t e = 0; e < n_dims; ++e) {
      if (e != dim && actual.shape_[e] != referencia[e]) {
        throw ShapeError("concat: el tensor en la posicion " +
                         std::to_string(t) + " tiene forma " +
                         forma_a_texto(actual.shape_) + " y no coincide con " +
                         forma_a_texto(referencia) + " en el eje " +
                         std::to_string(e));
      }
    }
    total_eje += actual.shape_[dim];
  }

  std::vector<std::size_t> forma_salida = referencia;
  forma_salida[dim] = total_eje;

  // El buffer es contiguo row-major, asi que se puede ver como
  // [externo][eje][interno]: para cada indice del bloque externo se pegan,
  // en orden, las porciones de cada tensor.
  std::size_t externo = 1;
  for (std::size_t e = 0; e < dim; ++e) externo *= referencia[e];
  std::size_t interno = 1;
  for (std::size_t e = dim + 1; e < n_dims; ++e) interno *= referencia[e];

  Tensor resultado(forma_salida);  // reserva memoria nueva, propia
  std::size_t escritos = 0;
  for (std::size_t bloque = 0; bloque < externo; ++bloque) {
    for (std::size_t t = 0; t < tensors.size(); ++t) {
      const Tensor& actual = tensors[t];
      const std::size_t elementos = actual.shape_[dim] * interno;
      std::memcpy(resultado.data_ + escritos,
                  actual.data_ + bloque * elementos,
                  elementos * sizeof(double));
      escritos += elementos;
    }
  }

  return resultado;  // devuelto por movimiento
}

}  // namespace tensorpp
