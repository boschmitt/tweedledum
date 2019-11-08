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

if (NOT TARGET percy)
  add_library(percy INTERFACE) # >v0.1.2
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
    target_compile_definitions(libabcsat PUBLIC ABC_NAMESPACE=pabc ABC_USE_NO_READLINE NOMINMAX)
    target_compile_definitions(libabcsat PUBLIC WIN32_NO_DLL)
  endif()
  target_link_libraries(percy INTERFACE libabcsat)
endif()

add_subdirectory(cudd)
