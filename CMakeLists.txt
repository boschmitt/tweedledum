# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt
# CMake build : tweedledum library
cmake_minimum_required(VERSION 3.8)
project(tweedledum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(UNIX)
  include(CheckCXXCompilerFlag)
  # show quite some warnings (but remove some intentionally)
  add_compile_options(-W -Wall -Wextra)
  foreach (WARNING unknown-pragmas gnu-anonymous-struct nested-anon-types)
    check_cxx_compiler_flag("-Wno-${WARNING}" HAS_WNO_${WARNING})
    if (HAS_WNO_${WARNING})
      add_compile_options(-Wno-${WARNING})
    endif()
  endforeach()
endif()

# Options
# =============================================================================
option(TWEEDLEDUM_EXAMPLES "Build examples" OFF)
option(TWEEDLEDUM_TESTS "Build tests" OFF)

# Libs
# =============================================================================
add_subdirectory(libs)

# Library
# =============================================================================
add_library(tweedledum INTERFACE)
target_include_directories(tweedledum INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tweedledum INTERFACE fmt kitty)

# Examples
# =============================================================================
if(TWEEDLEDUM_EXAMPLES)
  add_subdirectory(examples)
endif()

# Tests
# =============================================================================
if(TWEEDLEDUM_TESTS)
  add_subdirectory(tests)
endif()
