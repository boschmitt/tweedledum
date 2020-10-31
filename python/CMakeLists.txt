# Distributed under the MIT License (See accompanying file /LICENSE)
add_custom_target(pyTweedledum COMMENT "Build python bindings.")

pybind11_add_module(pyTweedledumIR
MODULE
    tweedledum/IR/Operators.cpp
    tweedledum/IR/Instruction.cpp
    tweedledum/IR/Circuit.cpp
    tweedledum/IR/IR.cpp
)
target_link_libraries(pyTweedledumIR PRIVATE tweedledum)
set_target_properties(pyTweedledumIR PROPERTIES OUTPUT_NAME libTweedledumIR)
add_dependencies(pyTweedledum pyTweedledumIR)

install(TARGETS pyTweedledumIR LIBRARY DESTINATION python/tweedledum/IR)

pybind11_add_module(pyTweedledumPasses
MODULE
    tweedledum/Passes/Passes.cpp
)
target_link_libraries(pyTweedledumPasses PRIVATE tweedledum)
set_target_properties(pyTweedledumPasses PROPERTIES OUTPUT_NAME libTweedledumPasses)
add_dependencies(pyTweedledum pyTweedledumPasses)

install(TARGETS pyTweedledumPasses LIBRARY DESTINATION python/tweedledum/Passes)

