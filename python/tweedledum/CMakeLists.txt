# Distributed under the MIT License (See accompanying file /LICENSE)
target_sources(_tweedledum PUBLIC
    bindings.cpp
    classical/kitty.cpp
    classical/mockturtle.cpp
    classical/utils.cpp
    ir/cbit.cpp
    ir/qubit.cpp
    ir/instruction.cpp
    ir/circuit.cpp
    operators/extension.cpp
    operators/ising.cpp
    operators/meta.cpp
    operators/standard.cpp
    passes/passes.cpp
    synthesis/synthesis.cpp
    target/device.cpp
    target/mapping.cpp
    utils/utils.cpp
)
