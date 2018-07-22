# Distributed under the MIT License (See accompanying file /LICENSE)

add_library(dum_fmt INTERFACE)
target_include_directories(dum_fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
target_compile_definitions(dum_fmt INTERFACE FMT_HEADER_ONLY)

add_library(kitty INTERFACE) # v0.4
target_include_directories(kitty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty)

add_library(tweedledee INTERFACE)
target_link_libraries(tweedledee INTERFACE c++experimental)
target_include_directories(tweedledee INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tweedledee)
