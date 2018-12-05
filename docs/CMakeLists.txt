# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt

find_program(DOXYGEN doxygen)
if (NOT DOXYGEN)
  message(STATUS "Target 'doc' disabled (requires doxygen)")
  return()
endif()

add_custom_target(doc
  COMMAND make html 
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  )