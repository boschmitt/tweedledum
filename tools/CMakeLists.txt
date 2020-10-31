# Distributed under the MIT License (See accompanying file /LICENSE)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

file(GLOB FILENAMES *.cpp)

foreach(filename ${FILENAMES})
    get_filename_component(basename ${filename} NAME_WE)
    add_executable(${basename} ${filename})
    target_link_libraries(${basename} PUBLIC tweedledum)
endforeach()