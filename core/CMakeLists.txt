cmake_minimum_required(VERSION 3.15)
message(STATUS "UNIX: ${UNIX}")
message(STATUS "APPLE: ${APPLE}")
message(STATUS "WIN32: ${WIN32}")


# Set the project (optional if set in root)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gather all source and header files recursively
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE CORE_HEADERS CONFIGURE_DEPENDS include/*.hpp)

add_library(core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)  # or gtk+-3.0 for GTK3

    # Add GTK include directories and compile flags to core target
    target_include_directories(core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GTK_INCLUDE_DIRS}
    )

    target_compile_options(core PUBLIC
        ${GTK_CFLAGS_OTHER}
    )

    # Link GTK libraries to core target
    target_link_libraries(core PUBLIC
        ${GTK_LIBRARIES}
    )
else()
    # On other platforms just add your core include directory
    target_include_directories(core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endif()

# Enable C++20 features
target_compile_features(core PUBLIC cxx_std_20)

# If using MSVC, enable multi-processor compilation
if (MSVC)
    target_compile_options(core PRIVATE /MP)
endif()
