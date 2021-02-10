# Distributed under the MIT License (See accompanying file /LICENSE)

target_sources(tweedledum PUBLIC
    ${TWEEDLEDUM_LIB_DIR}/Passes/Decomposition/barenco_decomp.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Decomposition/euler_decomp.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Decomposition/parity_decomp.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Mapping/Placer/JITPlacer.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Mapping/Placer/LinePlacer.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Mapping/Placer/SabrePlacer.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Mapping/Router/JITRouter.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Mapping/Router/SabreRouter.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Optimization/linear_resynth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Optimization/phase_folding.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/lhrs/lhrs_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/xag/xag_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/all_linear_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/decomp_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/diagonal_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/gray_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/linear_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/pkrm_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/pprm_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/spectrum_synth.cpp
    ${TWEEDLEDUM_LIB_DIR}/Passes/Synthesis/transform_synth.cpp
)
