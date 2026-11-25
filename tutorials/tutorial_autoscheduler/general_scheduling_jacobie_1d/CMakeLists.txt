cmake_minimum_required(VERSION 3.5)
project(General_scheduling)
 
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_VERBOSE_MAKEFILE "ON")
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
 
# Put the `generator` executable next to this file (retry11/),
# so it can be run as ../generator from retry11/build/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-rtti -std=c++11 -O0")
 
include_directories(
    $ENV{TIRAMISU_ROOT}/3rdParty/Halide/include
    $ENV{TIRAMISU_ROOT}/include/
    $ENV{TIRAMISU_ROOT}/3rdParty/isl/include
    $ENV{TIRAMISU_ROOT}/3rdParty/isl/build/include
)
 
link_directories(
    $ENV{TIRAMISU_ROOT}/build
    $ENV{TIRAMISU_ROOT}/3rdParty/Halide/lib
    $ENV{TIRAMISU_ROOT}/3rdParty/isl/build/lib
)
 
add_executable(generator generator.cpp)
 
# No auto scheduler: tiramisu_auto_scheduler is intentionally not linked.
target_link_libraries(generator tiramisu Halide isl dl z pthread)