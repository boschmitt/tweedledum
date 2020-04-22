# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : tweedledum library tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # I wish: -fno-rtti, but percy requires it :(

include_directories(catch2)

#  All Tests
#  =============================================================================
add_custom_target(tweedledum_tests COMMENT "Build all tests.")

set(tweedledum_tests_files
  "${CMAKE_CURRENT_SOURCE_DIR}/run_tests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/decomposition/barenco.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/decomposition/decompose.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/utility/to_logic_network.cpp"
  # "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/initial_map.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/placement/line_placement.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/placement/random_placement.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/placement/hsat_placement.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/jit_map.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/sabre_map.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/sat_map.cpp"
  # "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/mapping/zdd_map.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/optimization/gate_cancellation.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/optimization/phase_folding.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/oracles/xag_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/cnot_patel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/cnot_rz.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/dbs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/diagonal_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/esop_phase_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/gray_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/linear_synth.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/stg.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/swap_network.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/tbs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/transformation/asap_reschedule.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/gates/gate.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/io/dotqc.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/io/qasm.cpp"
  # "${CMAKE_CURRENT_SOURCE_DIR}/io/quil.cpp"
  # "${CMAKE_CURRENT_SOURCE_DIR}/io/write_projectq.cpp"
  # "${CMAKE_CURRENT_SOURCE_DIR}/io/write_qpic.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/networks/networks.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/networks/op_dag.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/networks/unitary.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/networks/wire.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/operations/operations.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/utils/angle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/views/layers_view.cpp"
  )

add_executable(run_tests "${tweedledum_tests_files}")
target_link_libraries(run_tests tweedledum)
add_dependencies(tweedledum_tests run_tests)
