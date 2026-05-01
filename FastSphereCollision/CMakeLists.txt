cmake_minimum_required(VERSION 3.15)
project(FastSphereCollision CXX)

add_executable(FastSphereCollision FastSphereCollision.cpp)

if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(FastSphereCollision PRIVATE -march=native)
endif()