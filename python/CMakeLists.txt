# Distributed under the MIT License (See accompanying file /LICENSE)

add_custom_target(py_tweedledum COMMENT "Build all tools.")

pybind11_add_module(core MODULE bindings/mockturtle.cpp
bindings/module.cpp bindings/tweedledum.cpp)
target_link_libraries(core PRIVATE tweedledum)
set_target_properties(core PROPERTIES OUTPUT_NAME core)

install(TARGETS core LIBRARY DESTINATION python/tweedledum)

add_dependencies(py_tweedledum core)
