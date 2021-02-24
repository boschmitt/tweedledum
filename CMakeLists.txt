# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : tweedledum library
cmake_minimum_required(VERSION 3.16)
project(tweedledum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(MSVC)
    add_compile_options(/Wall)
else()
    include(CheckCXXCompilerFlag)
    add_compile_options(-W -Wall -Wextra)
    foreach (WARNING unknown-pragmas gnu-anonymous-struct nested-anon-types)
        check_cxx_compiler_flag("-Wno-${WARNING}" HAS_WNO_${WARNING})
        if (HAS_WNO_${WARNING})
            add_compile_options(-Wno-${WARNING})
        endif()
    endforeach()
endif()

# Variables
# =============================================================================
set(TWEEDLEDUM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(TWEEDLEDUM_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

# Options
# =============================================================================
option(TWEEDLEDUM_EXAMPLES "Build examples" OFF)
option(TWEEDLEDUM_PYBINDS "Build python bindings" ON)
option(TWEEDLEDUM_TESTS "Build tests" OFF)
option(TWEEDLEDUM_TOOLS "Build tools" OFF)
option(TWEEDLEDUM_USE_EXTERNAL_FMT "Use an external fmt library" OFF)
option(TWEEDLEDUM_USE_EXTERNAL_JSON "Use an external JSON library" OFF)
option(TWEEDLEDUM_USE_EXTERNAL_PYBIND11 "Use an external pybind11 library" OFF)

option(ENABLE_BILL_Z3 "Enable Z3 interface for bill library" OFF)

# 3rd-party libraries
# =============================================================================
add_subdirectory(third_party)

# Library
# =============================================================================
add_library(tweedledum)
target_include_directories(tweedledum PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tweedledum eigen fmt::fmt-header-only mockturtle)

add_subdirectory(lib)

# Examples
# =============================================================================
if(TWEEDLEDUM_EXAMPLES)
  add_subdirectory(examples)
endif()

# Python bindings
# =============================================================================
if(TWEEDLEDUM_PYBINDS)
    add_subdirectory(python/tweedledum)
endif()

# Tests
# =============================================================================
if(TWEEDLEDUM_TESTS)
  add_subdirectory(test)
endif()

# Tools
# =============================================================================
if(TWEEDLEDUM_TOOLS)
    add_subdirectory(tools)
endif()
