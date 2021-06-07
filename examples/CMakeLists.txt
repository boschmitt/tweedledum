# Distributed under the MIT License (See accompanying file /LICENSE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

file(GLOB FILENAMES *.cpp)

foreach(filename ${FILENAMES})
    get_filename_component(basename ${filename} NAME_WE)
    add_executable("${basename}" ${filename})
    set_target_properties("${basename}" PROPERTIES OUTPUT_NAME ${basename})
    target_link_libraries("${basename}" PRIVATE tweedledum)
endforeach()