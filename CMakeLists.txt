# Distributed under the MIT License (See accompanying file /LICENSE )
# Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
# CMake build : tweedledum library
cmake_minimum_required(VERSION 3.8)
project(tweedledum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Options
# =============================================================================
option(TWEEDLEDUM_EXAMPLES "Build examples" OFF)

# Libs
# =============================================================================
add_subdirectory(libs)

# Library
# =============================================================================
add_library(tweedledum INTERFACE)
target_include_directories(tweedledum INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tweedledum INTERFACE tweedledee)

# Examples
# =============================================================================
if(TWEEDLEDUM_EXAMPLES)
  add_subdirectory(examples)
endif()
