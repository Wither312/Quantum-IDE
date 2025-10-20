add_executable(UnitTests
    test_FileSystem.cpp
)

target_include_directories(UnitTests PRIVATE
    ${PROJECT_SOURCE_DIR}/core/
)

target_link_libraries(UnitTests
    PRIVATE
        core
        Catch2::Catch2WithMain   # Use Catch2's main
)

# Optional: register tests automatically (for `ctest`)
include(CTest)
include(Catch)
catch_discover_tests(UnitTests)
