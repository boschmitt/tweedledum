# Distributed under the MIT License (See accompanying file /LICENSE)
pybind11_add_module(libPyTweedledum
MODULE
    bindings.cpp
    Classical/kitty.cpp
    Classical/mockturtle.cpp
    IR/Instruction.cpp
    IR/Circuit.cpp
    Operators/Extension.cpp
    Operators/Ising.cpp
    Operators/Meta.cpp
    Operators/Standard.cpp
    Passes/Passes.cpp
)
target_link_libraries(libPyTweedledum PRIVATE tweedledum)
install(TARGETS libPyTweedledum LIBRARY DESTINATION python/tweedledum)
