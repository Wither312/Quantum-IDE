cmake_minimum_required(VERSION 3.16)

project(QuantomIDE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_compile_options(/MP)
endif()

# Add core and glfw (assuming they have CMakeLists.txt files)

# ImGui sources
set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_opengl3.cpp
)
 find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)  # or gtk+-3.0 for GTK3

    include_directories(${GTK_INCLUDE_DIRS})
    link_directories(${GTK_LIBRARY_DIRS})
    add_definitions(${GTK_CFLAGS_OTHER})

    find_package(X11 REQUIRED)


# ImGui library
add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/json/single_include
)

find_package(OpenGL REQUIRED)

# Collect your application sources
file(GLOB_RECURSE APP_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(QuantomIDE ${APP_SOURCES})

target_include_directories(QuantomIDE PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/json/single_include
    ${CMAKE_SOURCE_DIR}/core/include
)

target_link_libraries(QuantomIDE PRIVATE
    imgui
    glfw
    OpenGL::GL
    core
    ${X11_LIBRARIES}
    ${GTK_LIBRARIES}

)
