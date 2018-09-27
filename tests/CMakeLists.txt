# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt
# CMake build : tweedledum library tests
add_compile_options(
  -fno-rtti
  -W
  -Wall
  -pedantic)

include_directories(catch2)

#  All Tests
#  =============================================================================
add_custom_target(tweedledum_tests COMMENT "Build all tests.")

set(tweedledum_tests_files
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/nct.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/optimization/single_qubit_gate_cancellation.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/cnot_patel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/decomposition_based.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/gray_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/remove_marked.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/io/quil.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/networks/dag_path.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/networks/gdg.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/networks/gates/qc_gate.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/views/slice_view.cpp")

add_executable(tweedledum_run_tests EXCLUDE_FROM_ALL "${tweedledum_tests_files}")
target_link_libraries(tweedledum_run_tests tweedledum)
add_dependencies(tweedledum_tests tweedledum_run_tests)
