# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >

add_library(tweedledee INTERFACE)
target_link_libraries(tweedledee INTERFACE c++experimental)
target_include_directories(tweedledee INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tweedledee)
