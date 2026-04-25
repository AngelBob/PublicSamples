cmake_minimum_required(VERSION 3.15)
project(FastSphereCollision CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/x64/${CMAKE_CFG_INTDIR})

add_executable(FastSphereCollision FastSphereCollision.cpp)

target_compile_definitions(FastSphereCollision PRIVATE $<$<CONFIG:Debug>:_DEBUG>$<$<CONFIG:Release>:NDEBUG>)
target_compile_options(FastSphereCollision PRIVATE $<$<CONFIG:Debug>:/Zi /Od /MDd>$<$<CONFIG:Release>:/O2 /MD /GL>)
target_link_options(FastSphereCollision PRIVATE $<$<CONFIG:Debug>:/DEBUG>$<$<CONFIG:Release>:/LTCG>)

if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(FastSphereCollision PRIVATE -march=native)
endif()