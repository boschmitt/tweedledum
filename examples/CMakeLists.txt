# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
add_compile_options(
  -fno-rtti
  -W
  -Wall
  -pedantic)

add_custom_target(examples COMMENT "Build all examples.")

add_executable(hello_world "hello_world.cpp")
add_dependencies(examples hello_world)

# Dumdee
# =============================================================================
set(dumdee_source_files "dumdee.cpp")
add_executable(dumdee "${dumdee_source_files}")
target_link_libraries(dumdee tweedledum)
add_dependencies(examples dumdee)
