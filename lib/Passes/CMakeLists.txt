# Distributed under the MIT License (See accompanying file /LICENSE)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Decomposition/barenco_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Decomposition/euler_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Decomposition/parity_decomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Placer/LinePlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Placer/RandomPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Placer/SatPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Placer/TrivialPlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/RePlacer/JitRePlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/RePlacer/SabreRePlacer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Router/JitRouter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mapping/Router/SabreRouter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Optimization/linear_resynth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Optimization/phase_folding.cpp
)

target_sources(tweedledum PUBLIC ${SRC})
if(TWEEDLEDUM_PYBINDS)
    target_sources(_tweedledum PUBLIC ${SRC})
endif()
