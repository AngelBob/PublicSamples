cmake_minimum_required(VERSION 3.15)
project(D3D12PerlinTriangle CXX)

# Source files
set(SOURCES
    D3D12PerlinTriangle.cpp
    Main.cpp
    Win32Application.cpp
    DXSample.cpp
    stdafx.cpp
)

set(HEADERS
    D3D12PerlinTriangle.h
    DXSample.h
    DXSampleHelper.h
    Win32Application.h
    stdafx.h
    stb_perlin.h
)

# Only build on Windows
if(WIN32)
    # This is a pure Windows application.
    add_executable(D3D12PerlinTriangle ${SOURCES} ${HEADERS})

    # Precompiled header
    target_precompile_headers(D3D12PerlinTriangle PRIVATE stdafx.h)

    # Windows subsystem
    set_target_properties(D3D12PerlinTriangle PROPERTIES
        LINKER_LANGUAGE CXX
        WIN32_EXECUTABLE FALSE
    )

    # D3D12 libraries
    target_link_libraries(D3D12PerlinTriangle PRIVATE
        d3d12.lib
        dxgi.lib
        d3dcompiler.lib
        dxguid.lib
    )

    set_target_properties(D3D12PerlinTriangle PROPERTIES
        LINK_FLAGS "/ENTRY:\"WinMainCRTStartup\""
    )

    # Configuration settings
    target_compile_definitions(D3D12PerlinTriangle PRIVATE
        WIN32
        _WINDOWS
        _UNICODE
        UNICODE
    )
endif()