# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt

if (NOT TARGET fmt)
  add_library(fmt INTERFACE)
  target_compile_definitions(fmt INTERFACE TWEEDLEDUM_FMT_INTERNAL FMT_HEADER_ONLY)
  target_include_directories(fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
endif()

if (NOT TARGET kitty)
  add_library(kitty INTERFACE) # v0.4
  target_include_directories(kitty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty)
endif()
