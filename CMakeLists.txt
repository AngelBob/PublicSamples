cmake_minimum_required(VERSION 3.15)
project(AllProjects)

# Collect all subproject names
set(SUBPROJECTS
    3DoorTest
    AarnioSequence
    AdventOfCode24
    AdventOfCode25
    D3D12PerlinTriangle
    DiamondPrinter
    FakeGuard
    FastSphereCollision
    InterviewProblems
    MemberHiding
    MostSignificantBit
    StlTypeTest
    SudokuSolver
    TemplateMetaProgramming
    TicTacToe
    UniqueLockTest
)

# Add all subprojects
foreach(SUBPROJECT ${SUBPROJECTS})
    add_subdirectory(${SUBPROJECT})
endforeach()

# Custom "clean" target for each subproject individually
foreach(SUBPROJECT ${SUBPROJECTS})
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