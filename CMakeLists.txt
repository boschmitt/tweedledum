# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : tweedledum library
cmake_minimum_required(VERSION 3.16)
project(tweedledum VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Options
# =============================================================================
option(TWEEDLEDUM_EXAMPLES "Build examples" OFF)
option(TWEEDLEDUM_PYBINDS "Build python bindings" ON)
option(TWEEDLEDUM_TESTS "Build tests" OFF)
option(TWEEDLEDUM_USE_EXTERNAL_PYBIND11 "Use an external pybind11 library" OFF)
option(TWEEDLEDUM_ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)
option(ENABLE_BILL_Z3 "Enable Z3 interface for bill library" OFF)

# Check if tweedledum is being used directly or via add_subdirectory
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR AND NOT DEFINED TWEEDLEDUM_MASTER_PROJECT)
    set(TWEEDLEDUM_MASTER_PROJECT ON)
else()
    set(TWEEDLEDUM_MASTER_PROJECT OFF)
endif()

if(SKBUILD)
    set(tweedledum_exclude EXCLUDE_FROM_ALL)
else()
    set(tweedledum_exclude "")
endif()

# 3rd-party libraries
# =============================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Eigen3 3.3 REQUIRED)
find_package(fmt 7.0.0 REQUIRED)
find_package(nlohmann_json 3.9.0 REQUIRED)
find_package(phmap 1.0.0 REQUIRED)
add_subdirectory(external)

# Python bindings
# =============================================================================
if(TWEEDLEDUM_PYBINDS AND TWEEDLEDUM_MASTER_PROJECT)
    pybind11_add_module(_tweedledum MODULE)
    target_include_directories(_tweedledum PRIVATE ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(_tweedledum PRIVATE
        Eigen3::Eigen3
        fmt::fmt-header-only
        mockturtle
        $<$<LINK_LANG_AND_ID:CXX,GNU>:stdc++fs>)
    target_compile_options(_tweedledum PRIVATE
        # clang/gcc warnings
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -W -Wall -Wextra>
        # MSVC 
        $<$<CXX_COMPILER_ID:MSVC>:/Wall /utf-8>)
    add_subdirectory(python/tweedledum)
    if(SKBUILD)
        install(TARGETS _tweedledum LIBRARY DESTINATION tweedledum)
    endif()
endif()

# Library
# =============================================================================
add_library(tweedledum ${tweedledum_exclude})
target_include_directories(tweedledum ${tweedledum_system} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(tweedledum PUBLIC
    Eigen3::Eigen3
    fmt::fmt-header-only
    mockturtle
    nlohmann_json
    $<$<LINK_LANG_AND_ID:CXX,GNU>:stdc++fs>)
target_compile_options(tweedledum PRIVATE
    # clang/gcc warnings
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
    -W -Wall -Wextra>
    # Covarege
    $<$<AND:$<BOOL:${TWEEDLEDUM_ENABLE_COVERAGE}>,$<CXX_COMPILER_ID:GNU>>:
    -O0 -g --coverage -fprofile-arcs -ftest-coverage>
    # MSVC 
    $<$<CXX_COMPILER_ID:MSVC>:/Wall /utf-8>)
add_subdirectory(src)

# Install
# =============================================================================
include(GNUInstallDirs)
install(TARGETS tweedledum ${tweedledum_exclude}
    EXPORT tweedledumConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ ${tweedledum_exclude}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# install(EXPORT tweedledumConfig DESTINATION share/tweedledum/cmake)
# export(TARGETS tweedledum mockturtle FILE tweedledumConfig.cmake)

# Examples
# =============================================================================
if(TWEEDLEDUM_EXAMPLES AND NOT SKBUILD)
  add_subdirectory(examples)
endif()

# Tests
# =============================================================================
if(TWEEDLEDUM_MASTER_PROJECT AND TWEEDLEDUM_TESTS AND NOT SKBUILD)
  add_subdirectory(tests)
endif()
