# Distributed under the MIT License (See accompanying file /LICENSE)
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
option(TWEEDLEDUM_DOCS "Build documentation" OFF)
option(TWEEDLEDUM_BLAS "Enable xtensor-blas functionality" OFF)
option(TWEEDLEDUM_EXAMPLES "Build examples" OFF)
option(TWEEDLEDUM_EXPERIMENTS "Build experiments" OFF)
option(TWEEDLEDUM_TESTS "Build tests" OFF)
option(TWEEDLEDUM_USE_EXTERNAL_CLI11 "Use an external CLI11 library" OFF)
option(TWEEDLEDUM_USE_EXTERNAL_FMT "Use an external fmt library" OFF)
option(TWEEDLEDUM_USE_EXTERNAL_JSON "Use an external JSON library" OFF)

# Libs
# =============================================================================
add_subdirectory(lib)

# Library
# =============================================================================
add_library(tweedledum INTERFACE)
target_include_directories(tweedledum INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tweedledum INTERFACE bill easy fmt glucose kitty
                                           nlohmann_json rang tweedledee)
if(TWEEDLEDUM_BLAS)
  target_link_libraries(tweedledum INTERFACE xtensor-blas)
endif()

# Documentation
# =============================================================================
if(TWEEDLEDUM_DOCS)
  add_subdirectory(docs)
endif()

# Examples
# =============================================================================
if(TWEEDLEDUM_EXAMPLES)
  add_subdirectory(examples)
endif()

# Experiments
# =============================================================================
if(TWEEDLEDUM_EXPERIMENTS)
  add_subdirectory(experiments)
endif()

# Tests
# =============================================================================
if(TWEEDLEDUM_TESTS)
  add_subdirectory(tests)
endif()
