# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt

if (NOT TARGET easy)
  add_library(easy INTERFACE)
  target_include_directories(easy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/easy)
endif()

if (NOT TARGET fmt)
  add_library(fmt INTERFACE)
  target_compile_definitions(fmt INTERFACE TWEEDLEDUM_FMT_INTERNAL FMT_HEADER_ONLY)
  target_include_directories(fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
endif()

if (NOT TARGET glucose)
  add_library(glucose INTERFACE)
  target_include_directories(glucose INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glucose)
endif()

if (NOT TARGET kitty)
  add_library(kitty INTERFACE)
  target_include_directories(kitty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty)
endif()

add_subdirectory(cudd)

add_library(qmdd
 qmdd/qmdd/QASMscanner.cpp
 qmdd/qmdd/QASMsimulator.cpp
 qmdd/qmdd/QASMtoken.cpp
 qmdd/qmdd/QMDDcircuit.cpp
 qmdd/qmdd/QMDDcomplexD.cpp
 qmdd/qmdd/QMDDpackage.cpp
 qmdd/qmdd/QMDDreorder.cpp
 qmdd/qmdd/Simulator.cpp
 qmdd/qmdd/qcost.cpp
 qmdd/qmdd/textFileUtilities.cpp
 qmdd/qmdd/timing.cpp)
target_include_directories(qmdd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/qmdd/qmdd)
target_include_directories(qmdd PUBLIC /usr/local/Cellar/mpfr/4.0.2/include)
target_include_directories(qmdd PUBLIC /usr/local/Cellar/gmp/6.1.2_2/include)