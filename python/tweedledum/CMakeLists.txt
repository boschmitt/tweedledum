# Distributed under the MIT License (See accompanying file /LICENSE)
add_custom_target(pyTweedledum COMMENT "Build python bindings.")

add_subdirectory(IR)
add_subdirectory(Operators)
add_subdirectory(Passes)

# I want to do it like this, but I failed misarably in trying to
# organize the project on the python side. For example, I want
# to do 'from tweedledum import IR' or 'from tweedledum import Ops'
# but to do that I seem to need the different libs :(

# pybind11_add_module(pyTweedledum
# MODULE
#     IR/Instruction.cpp
#     IR/Circuit.cpp
#     IR/IR.cpp
#     Operators/Extension.cpp
#     Operators/Ising.cpp
#     Operators/Meta.cpp
#     Operators/Standard.cpp
#     Passes/Passes.cpp
# )
# target_link_libraries(pyTweedledum PRIVATE tweedledum)
# set_target_properties(pyTweedledum PROPERTIES OUTPUT_NAME libTweedledum)

# install(TARGETS pyTweedledum LIBRARY DESTINATION python/tweedledum)
