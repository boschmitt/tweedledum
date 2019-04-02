# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt
add_compile_options(
  -fno-rtti
  -W
  -Wall
  -pedantic)

add_custom_target(examples COMMENT "Build all examples.")

file(GLOB FILENAMES *.cpp)

foreach(filename ${FILENAMES})
  get_filename_component(basename ${filename} NAME_WE)
  add_executable(${basename} ${filename})
  target_link_libraries(${basename} PUBLIC tweedledum)
  add_dependencies(examples ${basename})
endforeach()

target_link_libraries(cudd_tests PUBLIC obj)
target_include_directories(cudd_tests PUBLIC ../libs/cudd)
target_include_directories(cudd_tests PUBLIC ../libs/cudd/epd)
target_include_directories(cudd_tests PUBLIC ../libs/cudd/mtr)
target_include_directories(cudd_tests PUBLIC ../libs/cudd/st)
target_include_directories(cudd_tests PUBLIC ../libs/cudd/cudd)
target_include_directories(cudd_tests PUBLIC ${CMAKE_BINARY_DIR}/libs/cudd)
