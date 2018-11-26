# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt

if (NOT TARGET fmt)
  add_library(fmt INTERFACE)
  target_compile_definitions(fmt INTERFACE TWEEDLEDUM_FMT_INTERNAL FMT_HEADER_ONLY)
  target_include_directories(fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
endif()
