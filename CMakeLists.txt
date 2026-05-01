cmake_minimum_required(VERSION 3.15)
project(AllProjects)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/_BUILD/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/_BUILD/lib/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/_BUILD/obj/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)

# Collect all subproject names
# Iterate and add only directories
set(SUBPROJECT "")

file(GLOB project_dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} [^_.]*)
list(FILTER project_dirs EXCLUDE REGEX "^(TextAdventures)$")

foreach(subdir ${project_dirs})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
        list(APPEND SUBPROJECTS ${subdir})
    endif()
endforeach()

# Add all subprojects, default build settings, and custom clean targets
foreach(SUBPROJECT ${SUBPROJECTS})
    add_subdirectory(${SUBPROJECT})

    add_custom_target(clean_${SUBPROJECT}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/${SUBPROJECT} --target clean
        COMMENT "Cleaning ${SUBPROJECT}..."
    )
endforeach()

# Custom "clean_all" target that cleans everything
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMENT "Cleaning all projects..."
)

# Print available targets as a helper message at configure time
message(STATUS "")
message(STATUS "Available build targets:")
message(STATUS "  cmake --build . --target <project>   (build one project)")
message(STATUS "  cmake --build .                      (build all projects)")
message(STATUS "")
message(STATUS "Available clean targets:")
foreach(SUBPROJECT ${SUBPROJECTS})
    message(STATUS "  cmake --build . --target clean_${SUBPROJECT}")
endforeach()
message(STATUS "  cmake --build . --target clean_all")
message(STATUS "")