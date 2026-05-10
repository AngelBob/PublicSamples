cmake_minimum_required(VERSION 3.15)
project(FastSphereCollision CXX)

add_executable(FastSphereCollision FastSphereCollision.cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        target_compile_options(FastSphereCollision PRIVATE -march=native)
    endif()
endif()