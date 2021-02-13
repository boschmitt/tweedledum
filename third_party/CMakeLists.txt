# Distributed under the MIT License (See accompanying file /LICENSE)

if (NOT TARGET abcresub)
  add_library(abcresub INTERFACE)
  target_include_directories(abcresub INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/abcresub)
endif()

if (NOT TARGET libabcesop)
  set(STATIC_LIBABC true)
  add_subdirectory(abcesop)
endif()

# bill master branch: Nov.2020
if (NOT TARGET bill)
  add_library(bill INTERFACE)
  target_include_directories(bill SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/bill)

  if(ENABLE_BILL_Z3)
    target_compile_definitions(bill INTERFACE BILL_HAS_Z3)
    set(BILL_Z3_INCLUDE_PATH "" CACHE PATH "Path to Z3 includes, e.g., z3++.h")
    set(BILL_Z3_LIBRARY_PATH "" CACHE PATH "Path to Z3 library, e.g., libz3.a")
    if(NOT "${BILL_Z3_INCLUDE_PATH}" STREQUAL "")
      target_include_directories(bill SYSTEM INTERFACE ${BILL_Z3_INCLUDE_PATH})
    endif()
    if(NOT "${BILL_Z3_LIBRARY_PATH}" STREQUAL "")
      target_link_directories(bill INTERFACE ${BILL_Z3_LIBRARY_PATH})
    endif()
    if (WIN32)
      target_link_libraries(bill INTERFACE libz3)
    else()
      target_link_libraries(bill INTERFACE z3)
    endif()
  endif()
endif()

# Eigen 3.3.8
if (NOT TARGET eigen)
  add_library(eigen INTERFACE)
  target_include_directories(eigen SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/eigen)
endif()

# fmt 7.0.3
if(TWEEDLEDUM_USE_EXTERNAL_FMT)
  if (NOT TARGET fmt)
    find_package(fmt REQUIRED)
  endif()
else()
    add_subdirectory(fmt EXCLUDE_FROM_ALL)
endif()

# kitty master branch: Oct.2020
if (NOT TARGET kitty)
  add_library(kitty INTERFACE)
  target_include_directories(kitty SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty)
endif()

# rang master branch: Nov.2020
if (NOT TARGET rang)
  add_library(rang INTERFACE)
  target_include_directories(rang SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rang)
endif()

# lorina master branch: Feb.2021
if (NOT TARGET lorina)
  add_library(lorina INTERFACE)
  target_include_directories(lorina SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lorina)
  target_link_libraries(lorina INTERFACE rang fmt)
endif()

# nlohmann_json 3.9.1
if(TWEEDLEDUM_USE_EXTERNAL_JSON)
  find_package(nlohmann_json REQUIRED)
else()
  add_library(nlohmann_json INTERFACE)
  target_include_directories(nlohmann_json SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann)
endif()

# parallel_hashmap master branch: Nov.2020
if (NOT TARGET parallel_hashmap)
  add_library(parallel_hashmap INTERFACE) 
  target_include_directories(parallel_hashmap SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/parallel_hashmap)
endif()

# percy master branch: Nov.2020
if (NOT TARGET percy)
  add_library(percy INTERFACE)
  target_include_directories(percy SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/percy)
  target_compile_definitions(percy INTERFACE DISABLE_NAUTY)

  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(percy INTERFACE Threads::Threads)

  set(ABC_USE_NAMESPACE "pabc")
  set(STATIC_LIBABC true)
  add_subdirectory(abcsat)
  if (UNIX)
    target_compile_definitions(libabcsat PUBLIC "LIN64" ABC_NAMESPACE=pabc ABC_NO_USE_READLINE)
  elseif(WIN32)
    target_compile_definitions(libabcsat PUBLIC ABC_NAMESPACE=pabc ABC_USE_NO_READLINE NOMINMAX  WIN32_NO_DLL _CRT_SECURE_NO_WARNINGS)
  endif()
  target_link_libraries(percy INTERFACE libabcsat)
endif()

# mockturtle use_parallel_hashmap branch: Nov.2020
if (NOT TARGET mockturtle)
  add_library(mockturtle INTERFACE)
  target_include_directories(mockturtle SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mockturtle)
  target_link_libraries(mockturtle INTERFACE kitty lorina parallel_hashmap percy nlohmann_json bill libabcesop abcresub)
endif()

# pybind11 2.6.0
if(TWEEDLEDUM_USE_EXTERNAL_PYBIND11)
    if (NOT TARGET pybind11)
        find_package(pybind11 REQUIRED)
    endif()
else()
    add_subdirectory(pybind11 EXCLUDE_FROM_ALL)
endif()
