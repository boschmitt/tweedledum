# Distributed under the MIT License (See accompanying file /LICENSE)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Decomposition/barenco_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Decomposition/euler_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Decomposition/parity_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Placer/LinePlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Placer/RandomPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Placer/SATPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Placer/TrivialPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/RePlacer/JITRePlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/RePlacer/SabreRePlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Router/JITRouter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Router/SabreRouter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Optimization/linear_resynth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Optimization/phase_folding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/lhrs/lhrs_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/xag/xag_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/all_linear_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/decomp_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/diagonal_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/gray_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/linear_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/pkrm_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/pprm_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/spectrum_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Synthesis/transform_synth.cpp
)

target_sources(tweedledum PUBLIC ${SRC})
if(TWEEDLEDUM_PYBINDS)
    target_sources(_tweedledum PUBLIC ${SRC})
endif()
