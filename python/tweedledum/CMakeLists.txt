# Distributed under the MIT License (See accompanying file /LICENSE)
pybind11_add_module(_tweedledum
MODULE
    cpp_bindings/bindings.cpp
    cpp_bindings/Classical/kitty.cpp
    cpp_bindings/Classical/mockturtle.cpp
    cpp_bindings/Classical/utils.cpp
    cpp_bindings/IR/Instruction.cpp
    cpp_bindings/IR/Circuit.cpp
    cpp_bindings/Operators/Extension.cpp
    cpp_bindings/Operators/Ising.cpp
    cpp_bindings/Operators/Meta.cpp
    cpp_bindings/Operators/Standard.cpp
    cpp_bindings/Passes/Passes.cpp
)
target_link_libraries(_tweedledum PRIVATE tweedledum)
install(TARGETS _tweedledum LIBRARY DESTINATION python/tweedledum)
