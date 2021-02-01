# Distributed under the MIT License (See accompanying file /LICENSE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

file(GLOB FILENAMES *.cpp)

foreach(filename ${FILENAMES})
    get_filename_component(basename ${filename} NAME_WE)
    add_executable("examples_${basename}" ${filename})
    set_target_properties("examples_${basename}" PROPERTIES OUTPUT_NAME ${basename})
    target_link_libraries("examples_${basename}" PUBLIC tweedledum)
endforeach()