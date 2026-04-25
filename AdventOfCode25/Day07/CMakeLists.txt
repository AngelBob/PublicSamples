cmake_minimum_required(VERSION 3.15)
project(AoC25d07 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/x64/${CMAKE_CFG_INTDIR}/AoC25)

add_executable(AoC25d07 Day07.cpp)

target_compile_definitions(AoC25d07 PRIVATE $<$<CONFIG:Debug>:_DEBUG>$<$<CONFIG:Release>:NDEBUG>)
target_compile_options(AoC25d07 PRIVATE $<$<CONFIG:Debug>:/Zi /Od /MDd>$<$<CONFIG:Release>:/O2 /MD /GL>)
target_link_options(AoC25d07 PRIVATE $<$<CONFIG:Debug>:/DEBUG>$<$<CONFIG:Release>:/LTCG>)