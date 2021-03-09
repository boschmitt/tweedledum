# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : tweedledum library tests
include_directories(catch2)

set(tweedledum_tests_files
    ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/base/base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generators/adder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generators/less_than.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IR/Circuit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IR/Instruction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Operators/Unitary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Decomposition/barenco_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Decomposition/euler_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Optimization/phase_folding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Simulation/simulate_classically.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/all_linear_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/decomp_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/diagonal_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/gray_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/linear_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/pkrm_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/pprm_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/spectrum_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/transform_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Synthesis/xag_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/Angle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/BMatrix.cpp
)

add_executable(run_tests $<TARGET_OBJECTS:tweedledum> ${tweedledum_tests_files})
target_include_directories(run_tests PRIVATE 
    $<TARGET_PROPERTY:tweedledum,INCLUDE_DIRECTORIES>
)
target_link_libraries(run_tests PRIVATE 
    $<TARGET_PROPERTY:tweedledum,LINK_LIBRARIES>
)
