// tensor_core.cpp - Parte 1: ciclo de vida, memoria y fabricas.
//
// NOTA DE INTEGRACION: este archivo es la base minima que la Parte 2
// necesita para compilar y probarse. Si la Parte 1 del grupo ya tiene su
// propia version, reemplazar este archivo por ella: la Parte 2
// (tensor_ops.cpp y tensor_shape.cpp) solo depende de la interfaz
// declarada en include/tensor.h.

#include "tensor.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <ostream>
#include <random>
#include <sstream>

namespace tensorpp {

std::size_t Tensor::num_elements(const std::vector<std::size_t>& shape) {
  if (shape.empty()) {
    throw ShapeError("la forma debe tener al menos 1 dimension");
  }
  if (shape.size() > kMaxDims) {
    throw ShapeError("un tensor no puede tener mas de 3 dimensiones (recibio " +
                     std::to_string(shape.size()) + ")");
  }
  std::size_t total = 1;
  for (std::size_t dim : shape) {
    if (dim == 0) {
      throw ShapeError("las dimensiones deben ser mayores que cero");
    }
    total *= dim;
  }
  return total;
}

void Tensor::reset() noexcept {
  delete[] data_;
  data_ = nullptr;
  shape_.clear();
  size_ = 0;
}

void Tensor::steal_from(Tensor& other) noexcept {
  data_ = other.data_;
  shape_ = std::move(other.shape_);
  size_ = other.size_;
  other.data_ = nullptr;
  other.shape_.clear();
  other.size_ = 0;
}

// --- Construccion ---------------------------------------------------------

Tensor::Tensor() noexcept : data_(nullptr), shape_(), size_(0) {}

Tensor::Tensor(const std::vector<std::size_t>& shape,
               const std::vector<double>& values)
    : data_(nullptr), shape_(shape), size_(num_elements(shape)) {
  if (values.size() != size_) {
    throw ShapeError("la cantidad de valores (" +
                     std::to_string(values.size()) +
                     ") no coincide con el producto de las dimensiones (" +
                     std::to_string(size_) + ")");
  }
  data_ = new double[size_];
  std::copy(values.begin(), values.end(), data_);
}

Tensor::Tensor(const std::vector<std::size_t>& shape)
    : data_(nullptr), shape_(shape), size_(num_elements(shape)) {
  data_ = new double[size_]();  // el () inicializa en cero
}

// --- Ciclo de vida (regla de los cinco) -----------------------------------

Tensor::Tensor(const Tensor& other)
    : data_(nullptr), shape_(other.shape_), size_(other.size_) {
  if (size_ > 0) {
    data_ = new double[size_];
    std::memcpy(data_, other.data_, size_ * sizeof(double));
  }
}

Tensor::Tensor(Tensor&& other) noexcept : data_(nullptr), shape_(), size_(0) {
  steal_from(other);
}

Tensor& Tensor::operator=(const Tensor& other) {
  if (this == &other) {
    return *this;  // auto-asignacion: no liberar la memoria propia
  }
  double* nuevo = nullptr;
  if (other.size_ > 0) {
    // Se reserva antes de liberar: si `new` lanza, este tensor queda intacto.
    nuevo = new double[other.size_];
    std::memcpy(nuevo, other.data_, other.size_ * sizeof(double));
  }
  delete[] data_;
  data_ = nuevo;
  shape_ = other.shape_;
  size_ = other.size_;
  return *this;
}

Tensor& Tensor::operator=(Tensor&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  delete[] data_;
  data_ = nullptr;
  steal_from(other);
  return *this;
}

Tensor::~Tensor() { delete[] data_; }

// --- Fabricas -------------------------------------------------------------

Tensor Tensor::zeros(const std::vector<std::size_t>& shape) {
  return Tensor(shape);  // el constructor por forma ya inicializa en cero
}

Tensor Tensor::ones(const std::vector<std::size_t>& shape) {
  Tensor resultado(shape);
  std::fill(resultado.data_, resultado.data_ + resultado.size_, 1.0);
  return resultado;
}

Tensor Tensor::random(const std::vector<std::size_t>& shape, double min,
                      double max) {
  if (!(min < max)) {
    throw ShapeError("random requiere min < max");
  }
  // El generador es estatico para no reiniciar la secuencia en cada llamada.
  static std::mt19937 generador(std::random_device{}());
  std::uniform_real_distribution<double> distribucion(min, max);

  Tensor resultado(shape);
  for (std::size_t i = 0; i < resultado.size_; ++i) {
    resultado.data_[i] = distribucion(generador);
  }
  return resultado;
}

Tensor Tensor::arange(double start, double end, double step) {
  if (step == 0.0) {
    throw ShapeError("arange requiere un paso distinto de cero");
  }
  if ((step > 0.0 && start >= end) || (step < 0.0 && start <= end)) {
    throw ShapeError("arange: el rango indicado no produce ningun elemento");
  }
  // Cantidad de elementos: ceil((end - start) / step), igual que NumPy.
  const std::size_t total =
      static_cast<std::size_t>(std::ceil((end - start) / step));
  Tensor resultado(std::vector<std::size_t>{total});
  for (std::size_t i = 0; i < total; ++i) {
    resultado.data_[i] = start + step * static_cast<double>(i);
  }
  return resultado;
}

Tensor Tensor::clone() const { return Tensor(*this); }

// --- Accesores ------------------------------------------------------------

void Tensor::check_index_dims(std::size_t expected) const {
  if (ndim() != expected) {
    throw ShapeError("se indexo con " + std::to_string(expected) +
                     " indice(s) un tensor de " + std::to_string(ndim()) +
                     " dimension(es)");
  }
}

double& Tensor::at(std::size_t flat_index) {
  if (flat_index >= size_) {
    throw std::out_of_range("indice plano fuera de rango");
  }
  return data_[flat_index];
}

const double& Tensor::at(std::size_t flat_index) const {
  if (flat_index >= size_) {
    throw std::out_of_range("indice plano fuera de rango");
  }
  return data_[flat_index];
}

double& Tensor::operator()(std::size_t i) {
  check_index_dims(1);
  if (i >= shape_[0]) throw std::out_of_range("indice 0 fuera de rango");
  return data_[i];
}

double& Tensor::operator()(std::size_t i, std::size_t j) {
  check_index_dims(2);
  if (i >= shape_[0]) throw std::out_of_range("indice 0 fuera de rango");
  if (j >= shape_[1]) throw std::out_of_range("indice 1 fuera de rango");
  return data_[i * shape_[1] + j];
}

double& Tensor::operator()(std::size_t i, std::size_t j, std::size_t k) {
  check_index_dims(3);
  if (i >= shape_[0]) throw std::out_of_range("indice 0 fuera de rango");
  if (j >= shape_[1]) throw std::out_of_range("indice 1 fuera de rango");
  if (k >= shape_[2]) throw std::out_of_range("indice 2 fuera de rango");
  return data_[(i * shape_[1] + j) * shape_[2] + k];
}

const double& Tensor::operator()(std::size_t i) const {
  return const_cast<Tensor*>(this)->operator()(i);
}
const double& Tensor::operator()(std::size_t i, std::size_t j) const {
  return const_cast<Tensor*>(this)->operator()(i, j);
}
const double& Tensor::operator()(std::size_t i, std::size_t j,
                                 std::size_t k) const {
  return const_cast<Tensor*>(this)->operator()(i, j, k);
}

std::string Tensor::shape_str() const {
  if (shape_.empty()) return "()";
  std::ostringstream os;
  for (std::size_t i = 0; i < shape_.size(); ++i) {
    if (i > 0) os << "x";
    os << shape_[i];
  }
  return os.str();
}

std::ostream& operator<<(std::ostream& os, const Tensor& tensor) {
  os << "Tensor(" << tensor.shape_str() << ") [";
  const std::size_t maximo = 12;  // se recorta para no inundar la salida
  const std::size_t mostrar = std::min(tensor.size_, maximo);
  for (std::size_t i = 0; i < mostrar; ++i) {
    if (i > 0) os << ", ";
    os << tensor.data_[i];
  }
  if (tensor.size_ > maximo) os << ", ...";
  os << "]";
  return os;
}

}  // namespace tensorpp
