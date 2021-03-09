# Distributed under the MIT License (See accompanying file /LICENSE)
target_sources(_tweedledum PUBLIC
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
