# Distributed under the MIT License (See accompanying file /LICENSE)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

file(GLOB FILENAMES *.cpp)

foreach(filename ${FILENAMES})
    get_filename_component(basename ${filename} NAME_WE)
    add_executable(${basename} $<TARGET_OBJECTS:tweedledum> ${filename})
    target_include_directories(${basename}  PRIVATE 
        $<TARGET_PROPERTY:tweedledum,INCLUDE_DIRECTORIES>
    )
    target_link_libraries(${basename}  PRIVATE 
        $<TARGET_PROPERTY:tweedledum,LINK_LIBRARIES>
    )
endforeach()