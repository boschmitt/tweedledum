# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt

if (NOT TARGET easy)
  add_library(easy INTERFACE)
  target_include_directories(easy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/easy)
endif()

if (NOT TARGET fmt)
  add_library(fmt INTERFACE)
  target_include_directories(fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
  target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY)
endif()

if (NOT TARGET lorina)
  add_library(lorina INTERFACE)
  target_include_directories(lorina INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lorina)
endif()

if (NOT TARGET kitty)
  add_library(kitty INTERFACE) # v0.4
  target_include_directories(kitty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty)
endif()

if (NOT TARGET rang)
  add_library(rang INTERFACE)
  target_include_directories(rang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rang)
endif()

add_library(tweedledee INTERFACE)
target_link_libraries(tweedledee INTERFACE rang)
target_include_directories(tweedledee INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tweedledee)
