cmake_minimum_required(VERSION 3.15)
project(CUDASample LANGUAGES CXX CUDA)

add_executable(CUDASample cudasample.cu)

set_target_properties(CUDASample PROPERTIES
    CXX_STANDARD 20
    CUDA_ARCHITECTURES "native"
    LINK_FLAGS "/NODEFAULTLIB:libcmt.lib"
)