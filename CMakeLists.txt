# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : tweedledum library
cmake_minimum_required(VERSION 3.16)
project(tweedledum VERSION 1.1.0 LANGUAGES CXX C)

# CMP0116: Ninja generators transform `DEPFILE`s from `add_custom_command()`
# New in CMake 3.20. https://cmake.org/cmake/help/latest/policy/CMP0116.html
if(POLICY CMP0116)
    cmake_policy(SET CMP0116 OLD)
endif()

# Project setup and globals
# =============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Generate a CompilationDatabase (compile_commands.json file) for our build,
# for use by clang_complete, YouCompleteMe, etc.
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

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
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
find_package(Eigen3 3.3 REQUIRED)
find_package(fmt 8.1.1 REQUIRED)
find_package(nlohmann_json 3.9.0 REQUIRED)
find_package(phmap 1.0.0 REQUIRED)

add_subdirectory(external)

# MLIR/LLVM Configuration
# =============================================================================
find_package(MLIR REQUIRED CONFIG)

message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)
set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(TableGen)
include(AddLLVM)
include(AddMLIR)
include(HandleLLVMOptions)

# Installing the headers and docs needs to depend on generating any public
# tablegen'd targets.
add_custom_target(quirky-headers)
set_target_properties(quirky-headers PROPERTIES FOLDER "Misc")
add_custom_target(quirky-doc)

# Add MLIR and LLVM headers to the include path
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})

# Add QUIRKY and TWEEDLEDUM files to the include path
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)

link_directories(${LLVM_BUILD_LIBRARY_DIR})
add_definitions(${LLVM_DEFINITIONS})

# Define the default arguments to use with 'lit', and an option for the user to
# override.
set(LIT_ARGS_DEFAULT "-sv")
if (MSVC_IDE OR XCODE)
  set(LIT_ARGS_DEFAULT "${LIT_ARGS_DEFAULT} --no-progress-bar")
endif()
set(LLVM_LIT_ARGS "${LIT_ARGS_DEFAULT}" CACHE STRING "Default options for lit")

# QUIRKY configuration
# =============================================================================
set(QUIRKY_MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} ) # --src-root
set(QUIRKY_MAIN_INCLUDE_DIR ${QUIRKY_MAIN_SRC_DIR}/include)

set(QUIRKY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QUIRKY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(QUIRKY_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
set(QUIRKY_LIBRARY_DIR ${CMAKE_BINARY_DIR}/lib)
set(QUIRKY_TOOLS_DIR ${CMAKE_BINARY_DIR}/bin)

include(AddQUIRKY)

# Python bindings
# =============================================================================
if(TWEEDLEDUM_PYBINDS AND TWEEDLEDUM_MASTER_PROJECT)
    pybind11_add_module(_tweedledum MODULE)
    target_include_directories(_tweedledum PRIVATE ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(_tweedledum PRIVATE
        Eigen3::Eigen3
        fmt::fmt-header-only
        mockturtle
        $<$<CXX_COMPILER_ID:GNU>:stdc++fs>)
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
    $<$<CXX_COMPILER_ID:GNU>:stdc++fs>)
target_compile_options(tweedledum PRIVATE
    # clang/gcc warnings
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
    -W -Wall -Wextra>
    # Covarege
    $<$<AND:$<BOOL:${TWEEDLEDUM_ENABLE_COVERAGE}>,$<CXX_COMPILER_ID:GNU>>:
    -O0 -g --coverage -fprofile-arcs -ftest-coverage>
    # MSVC 
    $<$<CXX_COMPILER_ID:MSVC>:/Wall /utf-8>)



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


# Directory setup
# =============================================================================
add_subdirectory(include/tweedledum)
add_subdirectory(lib)
add_subdirectory(tools)

if(TWEEDLEDUM_EXAMPLES AND NOT SKBUILD)
  add_subdirectory(examples)
endif()

if(TWEEDLEDUM_MASTER_PROJECT AND TWEEDLEDUM_TESTS AND NOT SKBUILD)
  add_subdirectory(tests)
endif()
