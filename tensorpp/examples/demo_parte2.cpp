// demo_parte2.cpp - Ejemplos del enunciado correspondientes a la Parte 2.
// Reproduce las secciones 5 (operadores), 6 (view / unsqueeze) y 7 (concat).

#include "tensor.h"

#include <iostream>

using tensorpp::ShapeError;
using tensorpp::Tensor;

int main() {
  std::cout << "===== Seccion 5: sobrecarga de operadores =====\n";
  {
    Tensor A({2, 3}, {1, 2, 3, 4, 5, 6});
    Tensor B = Tensor::ones({2, 3});

    Tensor C = A + B;
    Tensor D = A - B;
    Tensor E = A * B;   // elemento a elemento (Hadamard)
    Tensor F = A * 2.0; // por escalar

    std::cout << "A = " << A << "\n";
    std::cout << "B = " << B << "\n";
    std::cout << "C = A + B   -> " << C << "\n";
    std::cout << "D = A - B   -> " << D << "\n";
    std::cout << "E = A * B   -> " << E << "\n";
    std::cout << "F = A * 2.0 -> " << F << "\n";
    std::cout << "A tras las operaciones (sin cambios) -> " << A << "\n";

    // Difusion: sumar un bias de 1x3 a una matriz de 2x3, como en el paso 4
    // de la red neuronal.
    Tensor bias({1, 3}, {100, 200, 300});
    std::cout << "A + bias(1x3) -> " << (A + bias) << "\n";

    // Dimensiones incompatibles -> excepcion.
    try {
      Tensor mala = A + Tensor::ones({4, 4});
    } catch (const ShapeError& e) {
      std::cout << "A + ones(4x4) lanza ShapeError: " << e.what() << "\n";
    }
  }

  std::cout << "\n===== Seccion 6.1: view =====\n";
  {
    Tensor A = Tensor::arange(0, 12);
    std::cout << "A = arange(0,12), size = " << A.shape_str() << "\n";

    const double* buffer = A.data();
    Tensor B = A.view({3, 4});
    std::cout << "B = A.view({3,4}), size = " << B.shape_str() << "\n";
    std::cout << "B = " << B << "\n";
    std::cout << "El buffer es el mismo (no hubo copia): "
              << (B.data() == buffer ? "si" : "no") << "\n";
    std::cout << "A quedo valido pero nulo (size = " << A.size() << ")\n";

    // Para conservar el original hay que copiarlo de forma explicita.
    Tensor X = Tensor::arange(0, 6);
    Tensor Y = X.clone().view({2, 3});
    std::cout << "X.clone().view({2,3}) -> Y = " << Y
              << " y X sigue con size = " << X.size() << "\n";
  }

  std::cout << "\n===== Seccion 6.2: unsqueeze =====\n";
  {
    Tensor A = Tensor::arange(0, 3);
    Tensor B = A.clone().unsqueeze(0);  // {1,3}
    Tensor C = A.clone().unsqueeze(1);  // {3,1}
    std::cout << "A = " << A << "\n";
    std::cout << "A.unsqueeze(0) -> forma " << B.shape_str() << "  " << B << "\n";
    std::cout << "A.unsqueeze(1) -> forma " << C.shape_str() << "  " << C << "\n";

    try {
      Tensor cubo({2, 2, 2}, {1, 2, 3, 4, 5, 6, 7, 8});
      cubo.unsqueeze(0);
    } catch (const ShapeError& e) {
      std::cout << "unsqueeze sobre un 3D lanza ShapeError: " << e.what() << "\n";
    }
  }

  std::cout << "\n===== Seccion 7: concat =====\n";
  {
    Tensor A = Tensor::ones({2, 3});
    Tensor B = Tensor::zeros({2, 3});

    Tensor C = Tensor::concat({A, B}, 0);
    Tensor D = Tensor::concat({A, B}, 1);
    std::cout << "concat({A,B}, 0) -> forma " << C.shape_str() << "  " << C << "\n";
    std::cout << "concat({A,B}, 1) -> forma " << D.shape_str() << "  " << D << "\n";

    try {
      Tensor mala = Tensor::concat({A, Tensor::ones({2, 4})}, 0);
    } catch (const ShapeError& e) {
      std::cout << "concat incompatible lanza ShapeError: " << e.what() << "\n";
    }
  }

  return 0;
}
