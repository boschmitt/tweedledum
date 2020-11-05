# Distributed under the MIT License (See accompanying file /LICENSE)

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

# master branch: 20.Oct.2020
if (NOT TARGET kitty)
  add_library(kitty INTERFACE)
  target_include_directories(kitty SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty)
endif()

# nlohmann_json 3.9.1
if(TWEEDLEDUM_USE_EXTERNAL_JSON)
  find_package(nlohmann_json REQUIRED)
else()
  add_library(nlohmann_json INTERFACE)
  target_include_directories(nlohmann_json SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann)
endif()

# pybind11 2.6.0
if(TWEEDLEDUM_USE_EXTERNAL_PYBIND11)
    if (NOT TARGET pybind11)
        find_package(pybind11 REQUIRED)
    endif()
else()
    add_subdirectory(pybind11 EXCLUDE_FROM_ALL)
endif()
