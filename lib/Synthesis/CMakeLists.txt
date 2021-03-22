# Distributed under the MIT License (See accompanying file /LICENSE)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/lhrs/lhrs_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xag/xag_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/a_star_swap_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/all_linear_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/decomp_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/diagonal_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gray_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/linear_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pkrm_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pprm_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spectrum_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/steiner_gauss_synth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/transform_synth.cpp
)

target_sources(tweedledum PUBLIC ${SRC})
if(TWEEDLEDUM_PYBINDS)
    target_sources(_tweedledum PUBLIC ${SRC})
endif()
