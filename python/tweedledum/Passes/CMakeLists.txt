# Distributed under the MIT License (See accompanying file /LICENSE)

pybind11_add_module(pyTweedledumPasses
MODULE
    Passes.cpp
)
target_link_libraries(pyTweedledumPasses PRIVATE tweedledum)
set_target_properties(pyTweedledumPasses PROPERTIES OUTPUT_NAME libTweedledumPasses)
add_dependencies(pyTweedledum pyTweedledumPasses)

install(TARGETS pyTweedledumPasses LIBRARY DESTINATION python/tweedledum/Passes)
