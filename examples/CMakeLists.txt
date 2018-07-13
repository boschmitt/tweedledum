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
