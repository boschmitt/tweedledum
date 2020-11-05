# Distributed under the MIT License (See accompanying file /LICENSE)

pybind11_add_module(pyTweedledumIR
MODULE
    Instruction.cpp
    Circuit.cpp
    IR.cpp
)
target_link_libraries(pyTweedledumIR PRIVATE tweedledum)
set_target_properties(pyTweedledumIR PROPERTIES OUTPUT_NAME libTweedledumIR)
add_dependencies(pyTweedledum pyTweedledumIR)

install(TARGETS pyTweedledumIR LIBRARY DESTINATION python/tweedledum/IR)
