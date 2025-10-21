cmake_minimum_required(VERSION 3.15)
project(QuantomIDE)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add core directory
add_subdirectory(core)
add_subdirectory(vendor/glfw)

add_subdirectory(vendor/Catch2)
# Add tests directory
add_subdirectory(tests)

add_subdirectory(QuantomIDE)
