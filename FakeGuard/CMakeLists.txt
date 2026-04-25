cmake_minimum_required(VERSION 3.15)
project(FakeGuard CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/x64/${CMAKE_CFG_INTDIR})

# Source files
set(SOURCES
    FakeGuard.cpp
)

# Resources
set(RESOURCES
    FakeGuard.rc
)

if(WIN32)
    # This is a pure Windows application.
    add_executable(FakeGuard ${SOURCES})

    target_sources(FakeGuard PRIVATE ${RESOURCES})

    set_target_properties(FakeGuard PROPERTIES
        LINK_FLAGS "/ENTRY:\"wWinMainCRTStartup\""
    )

    # Configuration settings
    target_compile_definitions(FakeGuard PRIVATE
        WIN32
        _UNICODE
        UNICODE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )

    target_compile_options(FakeGuard PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od /MDd>
        $<$<CONFIG:Release>:/O2 /MD /GL>
    )

    target_link_options(FakeGuard PRIVATE
        $<$<CONFIG:Debug>:/DEBUG>
        $<$<CONFIG:Release>:/LTCG>
    )
endif()
