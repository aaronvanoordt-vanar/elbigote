# Makefile de Tensor++
#
#   make          -> compila las pruebas y el demo
#   make test     -> compila y ejecuta las pruebas de la Parte 2
#   make demo     -> compila y ejecuta los ejemplos del enunciado
#   make asan     -> ejecuta las pruebas con AddressSanitizer (fugas/usos
#                    invalidos de memoria)
#   make clean    -> borra los binarios y objetos

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
INCLUDES  = -Iinclude
BUILD     = build

FUENTES = src/tensor_core.cpp src/tensor_ops.cpp src/tensor_shape.cpp

.PHONY: all test demo asan clean

all: $(BUILD)/test_parte2 $(BUILD)/demo_parte2

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/test_parte2: $(FUENTES) tests/test_parte2.cpp include/tensor.h | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(FUENTES) tests/test_parte2.cpp -o $@

$(BUILD)/demo_parte2: $(FUENTES) examples/demo_parte2.cpp include/tensor.h | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(FUENTES) examples/demo_parte2.cpp -o $@

test: $(BUILD)/test_parte2
	./$(BUILD)/test_parte2

demo: $(BUILD)/demo_parte2
	./$(BUILD)/demo_parte2

# Comprueba que view/unsqueeze/concat no dejan fugas ni dobles liberaciones.
asan: | $(BUILD)
	$(CXX) -std=c++17 -Wall -Wextra -g -O1 -fsanitize=address,undefined \
	  $(INCLUDES) $(FUENTES) tests/test_parte2.cpp -o $(BUILD)/test_asan
	./$(BUILD)/test_asan

clean:
	rm -rf $(BUILD)
