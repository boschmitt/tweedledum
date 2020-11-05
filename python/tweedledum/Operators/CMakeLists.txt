# Distributed under the MIT License (See accompanying file /LICENSE)

pybind11_add_module(pyTweedledumOp
MODULE
    Extension.cpp
    Ising.cpp
    Meta.cpp
    Operators.cpp
    Standard.cpp
)
target_link_libraries(pyTweedledumOp PRIVATE tweedledum)
set_target_properties(pyTweedledumOp PROPERTIES OUTPUT_NAME libTweedledumOp)
add_dependencies(pyTweedledum pyTweedledumOp)

install(TARGETS pyTweedledumOp LIBRARY DESTINATION python/tweedledum/Operators)
