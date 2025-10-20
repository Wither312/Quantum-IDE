# core/CMakeLists.txt

cmake_minimum_required(VERSION 3.15)

# Set the project (optional if set in root)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gather all source and header files recursively
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE CORE_HEADERS CONFIGURE_DEPENDS include/*.hpp)

add_library(core STATIC ${CORE_SOURCES} ${CORE_HEADERS})


# Create static library


# Add include path (recursive, starting from "include/")
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Enable C++20
target_compile_features(core PUBLIC cxx_std_20)

# If using MSVC, enable multi-processor compilation
if (MSVC)
    target_compile_options(core PRIVATE /MP)
endif()
