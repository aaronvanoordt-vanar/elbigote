// tensor.h - Libreria Tensor++ (Programacion III, Tarea #2)
//
// Declaracion de la clase Tensor (maximo 3 dimensiones, almacenamiento
// contiguo en memoria dinamica propia mediante `double*`).
//
// Reparto del grupo:
//   Parte 1 (Bryan)     : ciclo de vida, memoria y constructores estaticos -> src/tensor_core.cpp
//   Parte 2 (Dylanvoa14): sobrecarga de operadores                         -> src/tensor_ops.cpp
//                         view / unsqueeze / concat                        -> src/tensor_shape.cpp
//   Parte 3 (Johnb1701) : dot / matmul / red neuronal / README / tests
#ifndef TENSORPP_TENSOR_H
#define TENSORPP_TENSOR_H

#include <cstddef>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <vector>

namespace tensorpp {

// Excepcion lanzada cuando las dimensiones de una operacion no son
// compatibles (suma de formas distintas, view con otro numero de
// elementos, concat sobre un eje invalido, etc.).
class ShapeError : public std::invalid_argument {
 public:
  explicit ShapeError(const std::string& msg) : std::invalid_argument(msg) {}
};

class Tensor {
 public:
  // La tarea limita los tensores a 1D, 2D o 3D.
  static const std::size_t kMaxDims = 3;

  // ---------------------------------------------------------------------
  // Parte 1: construccion, ciclo de vida y fabricas
  // ---------------------------------------------------------------------
  Tensor() noexcept;
  Tensor(const std::vector<std::size_t>& shape,
         const std::vector<double>& values);
  // Tensor con la forma dada e inicializado en cero. Se usa internamente
  // como buffer de salida de los operadores y de concat.
  explicit Tensor(const std::vector<std::size_t>& shape);

  Tensor(const Tensor& other);                 // copia profunda
  Tensor(Tensor&& other) noexcept;             // movimiento
  Tensor& operator=(const Tensor& other);      // asignacion por copia
  Tensor& operator=(Tensor&& other) noexcept;  // asignacion por movimiento
  ~Tensor();

  static Tensor zeros(const std::vector<std::size_t>& shape);
  static Tensor ones(const std::vector<std::size_t>& shape);
  static Tensor random(const std::vector<std::size_t>& shape, double min,
                       double max);
  static Tensor arange(double start, double end, double step = 1.0);

  // ---------------------------------------------------------------------
  // Parte 2: sobrecarga de operadores  (src/tensor_ops.cpp)
  // ---------------------------------------------------------------------
  // Operaciones elemento a elemento. Ninguna modifica a los operandos y
  // todas devuelven un tensor nuevo con memoria propia.
  //
  // Las formas deben ser compatibles: iguales, o difundibles (broadcast)
  // al estilo NumPy alineando las dimensiones por la derecha, donde una
  // dimension de tamano 1 se repite. Esto es lo que permite el paso
  // "sumar un bias de 1x100 a un tensor de 1000x100" de la red neuronal.
  // Si no son compatibles se lanza ShapeError.
  Tensor operator+(const Tensor& other) const;
  Tensor operator-(const Tensor& other) const;
  Tensor operator*(const Tensor& other) const;  // producto Hadamard
  Tensor operator*(double scalar) const;

  // Version simetrica para poder escribir `2.0 * A`.
  friend Tensor operator*(double scalar, const Tensor& tensor);

  // Operadores compuestos: modifican *este* tensor in-place. La forma del
  // otro operando debe poder difundirse sobre la forma de este.
  Tensor& operator+=(const Tensor& other);
  Tensor& operator-=(const Tensor& other);
  Tensor& operator*=(const Tensor& other);
  Tensor& operator*=(double scalar);

  // Negacion unaria (util para escribir A + (-B)).
  Tensor operator-() const;

  // Comparacion exacta elemento a elemento (misma forma y mismos valores).
  bool operator==(const Tensor& other) const;
  bool operator!=(const Tensor& other) const;

  // ---------------------------------------------------------------------
  // Parte 2: modificacion de dimensiones  (src/tensor_shape.cpp)
  // ---------------------------------------------------------------------
  // Reinterpreta la forma del tensor SIN copiar los datos: el buffer se
  // traslada al tensor resultante. Tras la llamada este tensor queda en un
  // estado valido pero nulo (empty() == true), igual que el origen de un
  // movimiento. Requiere que el numero total de elementos no cambie y que
  // la nueva forma no exceda 3 dimensiones.
  Tensor view(const std::vector<std::size_t>& shape);

  // Inserta una dimension de tamano 1 en la posicion `dim` (0 <= dim <=
  // ndim()). Tampoco copia los datos: los traslada al tensor resultante.
  // El total de dimensiones resultante no puede exceder 3.
  Tensor unsqueeze(std::size_t dim);

  // Une varios tensores a lo largo de la dimension `dim`. Todos deben
  // tener el mismo numero de dimensiones y coincidir en todos los ejes
  // salvo en `dim`. Reserva memoria nueva, copia los datos de forma
  // controlada y devuelve el resultado por movimiento.
  static Tensor concat(const std::vector<Tensor>& tensors, std::size_t dim);

  // Copia profunda explicita. `view` y `unsqueeze` vacian el tensor
  // original, asi que `A.clone().unsqueeze(1)` permite reinterpretar la
  // forma conservando `A` con sus datos.
  Tensor clone() const;

  // ---------------------------------------------------------------------
  // Accesores
  // ---------------------------------------------------------------------
  std::size_t size() const noexcept { return size_; }
  std::size_t ndim() const noexcept { return shape_.size(); }
  const std::vector<std::size_t>& shape() const noexcept { return shape_; }
  bool empty() const noexcept { return size_ == 0; }
  const double* data() const noexcept { return data_; }
  double* data() noexcept { return data_; }

  // Acceso por indices logicos (con validacion de rango y de dimension).
  double& operator()(std::size_t i);
  double& operator()(std::size_t i, std::size_t j);
  double& operator()(std::size_t i, std::size_t j, std::size_t k);
  const double& operator()(std::size_t i) const;
  const double& operator()(std::size_t i, std::size_t j) const;
  const double& operator()(std::size_t i, std::size_t j, std::size_t k) const;

  // Acceso plano al buffer contiguo (orden row-major).
  double& at(std::size_t flat_index);
  const double& at(std::size_t flat_index) const;

  // Forma en texto, p.ej. "1000x400". Util para el `size` que la tarea
  // pide imprimir en cada etapa de la red neuronal.
  std::string shape_str() const;

  friend std::ostream& operator<<(std::ostream& os, const Tensor& tensor);

  // Parte 3: implementadas en src/tensor_linalg.cpp
  friend Tensor dot(const Tensor& a, const Tensor& b);
  friend Tensor matmul(const Tensor& a, const Tensor& b);

  // Numero de elementos que describe una forma. Lanza ShapeError si la
  // forma es invalida (0 dimensiones, mas de 3, o alguna dimension nula).
  static std::size_t num_elements(const std::vector<std::size_t>& shape);

 private:
  double* data_;
  std::vector<std::size_t> shape_;
  std::size_t size_;

  // Libera el buffer y deja el tensor en el estado valido pero nulo.
  void reset() noexcept;
  // Toma posesion del buffer de `other`, que queda valido pero nulo.
  void steal_from(Tensor& other) noexcept;
  void check_index_dims(std::size_t expected) const;
};

std::ostream& operator<<(std::ostream& os, const Tensor& tensor);
Tensor operator*(double scalar, const Tensor& tensor);

// Parte 3: producto punto y producto matricial (src/tensor_linalg.cpp).
Tensor dot(const Tensor& a, const Tensor& b);
Tensor matmul(const Tensor& a, const Tensor& b);

}  // namespace tensorpp

#endif  // TENSORPP_TENSOR_H
