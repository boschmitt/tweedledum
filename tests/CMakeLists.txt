# Distributed under the MIT License (See accompanying file /LICENSE)
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
  "${CMAKE_CURRENT_SOURCE_DIR}/run_tests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/decomposition/barenco.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/decomposition/dt.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/sabre_map.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/zdd_map.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/optimization/gate_cancellation.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/optimization/phase_folding.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/cnot_patel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/dbs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/diagonal_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/esop_phase_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/gray_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/linear_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/stg.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/tbs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/identify_rz.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/gates/gates.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/io/dotqc.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/io/qasm.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/io/quil.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/io/write_projectq.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/io/write_qpic.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/networks/networks.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/networks/io_id.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/utils/angle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/views/layers_view.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/views/pathsum_view.cpp"
  )

if(TWEEDLEDUM_BLAS)
  set(tweedledum_tests_files
    "${tweedledum_tests_files}"
    "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/zyz.cpp"
    )
endif()

add_executable(run_tests "${tweedledum_tests_files}")
target_link_libraries(run_tests tweedledum)
add_dependencies(tweedledum_tests run_tests)
