# Distributed under the MIT License (See accompanying file /LICENSE)
pybind11_add_module(_tweedledum
MODULE
    bindings.cpp
    classical/kitty.cpp
    classical/mockturtle.cpp
    classical/utils.cpp
    ir/instruction.cpp
    ir/circuit.cpp
    operators/extension.cpp
    operators/ising.cpp
    operators/meta.cpp
    operators/standard.cpp
    passes/passes.cpp
)
target_link_libraries(_tweedledum PRIVATE tweedledum)
if(SKBUILD)
    install(TARGETS _tweedledum LIBRARY DESTINATION python/tweedledum)
endif()
