# Distributed under the MIT License (See accompanying file /LICENSE)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/QASM/Lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QASM/Parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qasm.cpp
)

target_sources(tweedledum PUBLIC ${SRC})
if(TWEEDLEDUM_PYBINDS)
    target_sources(_tweedledum PUBLIC ${SRC})
endif()
