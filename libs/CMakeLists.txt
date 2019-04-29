# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt

if (NOT TARGET easy)
  add_library(easy INTERFACE)
  target_include_directories(easy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/easy)
endif()

if (NOT TARGET fmt)
  add_library(fmt INTERFACE)
  target_compile_definitions(fmt INTERFACE TWEEDLEDUM_FMT_INTERNAL FMT_HEADER_ONLY)
  target_include_directories(fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
endif()

if (NOT TARGET glucose)
  add_library(glucose INTERFACE)
  target_include_directories(glucose INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glucose)
endif()

if (NOT TARGET kitty)
  add_library(kitty INTERFACE)
  target_include_directories(kitty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty)
endif()

if(TWEEDLEDUM_BLAS)
  list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/xtl-0.6.2)
  list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/xtensor-0.20.4)
  add_subdirectory(xtl-0.6.2)
  add_subdirectory(xtensor-0.20.4)
  add_subdirectory(xtensor-blas-0.16.0)
  add_library(xtensor-blas INTERFACE)
  target_include_directories(xtensor-blas INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/xtensor-blas-0.16.0/include)
  target_link_libraries(xtensor-blas INTERFACE xtensor)

  add_definitions(-DHAVE_CBLAS=1)

  if (WIN32)
    find_package(OpenBLAS REQUIRED)
    set(BLAS_LIBRARIES ${CMAKE_INSTALL_PREFIX}${OpenBLAS_LIBRARIES})
  else()
    find_package(BLAS REQUIRED)
    find_package(LAPACK REQUIRED)
  endif()

  message(STATUS "BLAS VENDOR:    " ${BLA_VENDOR})
  message(STATUS "BLAS LIBRARIES: " ${BLAS_LIBRARIES})

  target_link_libraries(xtensor-blas INTERFACE ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
endif()
