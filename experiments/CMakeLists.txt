# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt
add_compile_options(
  -W
  -Wall
  -pedantic)

add_custom_target(experiments COMMENT "Build all experiments.")

macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      if (EXISTS "${curdir}/${child}/CMakeLists.txt")
        list(APPEND dirlist ${child})
      endif()
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

SUBDIRLIST(subdirs ${CMAKE_CURRENT_SOURCE_DIR})
foreach(subdir ${subdirs})
  add_subdirectory(${subdir})
endforeach()

