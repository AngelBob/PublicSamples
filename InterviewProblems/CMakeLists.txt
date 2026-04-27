cmake_minimum_required(VERSION 3.15)
project(InterviewProblems C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/x64/${CMAKE_CFG_INTDIR})

add_executable(InterviewProblems
    InterviewProblems.c
    SortLists.c
    HistogramArea.c
)

target_compile_definitions(InterviewProblems PRIVATE $<$<CONFIG:Debug>:_DEBUG>$<$<CONFIG:Release>:NDEBUG>)
target_compile_options(InterviewProblems PRIVATE $<$<CONFIG:Debug>:/Zi /Od /MDd>$<$<CONFIG:Release>:/O2 /MD /GL>)
target_link_options(InterviewProblems PRIVATE $<$<CONFIG:Debug>:/DEBUG>$<$<CONFIG:Release>:/LTCG>)