# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : tweedledum library
cmake_minimum_required(VERSION 3.8)
project(tweedledum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra")
endif()

# Options
# =============================================================================
option(TWEEDLEDUM_PYBINDS "Build python bindings" ON)
option(TWEEDLEDUM_TESTS "Build tests" OFF)
option(TWEEDLEDUM_TOOLS "Build tools" OFF)
option(TWEEDLEDUM_USE_EXTERNAL_FMT "Use an external fmt library" OFF)

# 3rd-party libraries
# =============================================================================
add_subdirectory(third_party)

# Library
# =============================================================================
add_library(tweedledum INTERFACE)
target_include_directories(tweedledum INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tweedledum INTERFACE fmt mockturtle nlohmann_json)

# Python bindings
# =============================================================================
if(TWEEDLEDUM_PYBINDS)
  add_subdirectory(python)
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
