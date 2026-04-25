cmake_minimum_required(VERSION 3.15)
project(AoC24d25 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/x64/${CMAKE_CFG_INTDIR}/AoC24)

add_executable(AoC24d25 Day25.cpp)

target_compile_definitions(AoC24d25 PRIVATE $<$<CONFIG:Debug>:_DEBUG>$<$<CONFIG:Release>:NDEBUG>)
target_compile_options(AoC24d25 PRIVATE $<$<CONFIG:Debug>:/Zi /Od /MDd>$<$<CONFIG:Release>:/O2 /MD /GL>)
target_link_options(AoC24d25 PRIVATE $<$<CONFIG:Debug>:/DEBUG>$<$<CONFIG:Release>:/LTCG>)