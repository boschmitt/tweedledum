# Distributed under the MIT License (See accompanying file /LICENSE)

set(SRC
    # Parser
    ${CMAKE_CURRENT_SOURCE_DIR}/Parser/QASM/Lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Parser/QASM/Parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Parser/qasm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Parser/tfc.cpp
    # Passes
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Decomposition/barenco_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Decomposition/bridge_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Decomposition/euler_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Decomposition/parity_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/Placer/ApprxSatPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/Placer/LinePlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/Placer/RandomPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/Placer/SatPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/Placer/TrivialPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/RePlacer/JitRePlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/RePlacer/SabreRePlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/Router/BridgeRouter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/Router/JitRouter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Mapping/Router/SabreRouter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Optimization/gate_cancellation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Optimization/linear_resynth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Optimization/phase_folding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Passes/Optimization/steiner_resynth.cpp
    # Synthesis
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/lhrs/lhrs_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/xag/xag_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/a_star_swap_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/all_linear_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/cx_dihedral_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/decomp_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/diagonal_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/gray_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/linear_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/pkrm_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/pprm_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/spectrum_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/steiner_gauss_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/transform_synth.cpp
    # Target
    ${CMAKE_CURRENT_SOURCE_DIR}/Target/Device.cpp
    # Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/Visualization/string_utf8.cpp
)

target_sources(tweedledum PRIVATE ${SRC})
if(TWEEDLEDUM_PYBINDS)
    target_sources(_tweedledum PRIVATE ${SRC})
endif()
