# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : tweedledum library tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # I wish: -fno-rtti, but percy requires it :(

include_directories(catch2)

#  All Tests
#  =============================================================================
add_custom_target(tweedledum_tests COMMENT "Build all tests.")

set(tweedledum_tests_files
  "${CMAKE_CURRENT_SOURCE_DIR}/run_tests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/simulation/simulate_classically.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/decomp_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/diagonal_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/pkrm_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/pprm_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/transform_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/xag_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/generators/adder.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/generators/less_than.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ir/unitary.cpp"
  )

add_executable(run_tests "${tweedledum_tests_files}")
target_link_libraries(run_tests tweedledum)
add_dependencies(tweedledum_tests run_tests)
