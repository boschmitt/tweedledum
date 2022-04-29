# Distributed under the MIT License (See accompanying file /LICENSE)
include_directories(catch2)

get_filename_component(TEST_QASM_DIR qasm ABSOLUTE)
add_compile_definitions(TEST_QASM_DIR="${TEST_QASM_DIR}")

add_subdirectory(quirky)
add_subdirectory(tweedledum)
