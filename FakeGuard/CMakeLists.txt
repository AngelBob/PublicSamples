cmake_minimum_required(VERSION 3.15)
project(FakeGuard CXX)

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
    )
endif()
