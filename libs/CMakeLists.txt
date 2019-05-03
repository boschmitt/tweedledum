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

if (NOT TARGET rang)
  add_library(rang INTERFACE)
  target_include_directories(rang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rang)
endif()

if (NOT TARGET tweedledee)
  add_library(tweedledee INTERFACE)
  target_include_directories(tweedledee INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tweedledee)
endif()
