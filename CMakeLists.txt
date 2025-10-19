cmake_minimum_required(VERSION 3.16)

project(QuantomIDE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_compile_options(/MP)
endif()

# ImGui sources
set(IMGUI_SOURCES
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp 
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES} "src/Application.cpp"  "src/EditorManager.cpp" "src/BuildSystem.cpp" "src/Debugger.cpp" "src/UIManager.cpp" 
"src/SettingsManager.cpp" "src/FileWatcher.cpp" "src/FileManager.cpp" "src/MenuBar.cpp"  "src/TreeView.cpp" "src/Project.cpp")

target_include_directories(imgui PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/vendor/imgui
    ${PROJECT_SOURCE_DIR}/vendor/imgui/backends
    ${PROJECT_SOURCE_DIR}/vendor/glfw/include
    # In your main CMakeLists.txt
    ${CMAKE_SOURCE_DIR}/vendor/json/single_include

)

# GLFW from submodule
add_subdirectory(vendor/glfw)

find_package(OpenGL REQUIRED)

# Collect your app source files (only src/)
file(GLOB_RECURSE APP_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(QuantomIDE ${APP_SOURCES} "src/Application.cpp"  "src/EditorManager.cpp" "src/BuildSystem.cpp" "src/Debugger.cpp" "src/UIManager.cpp" "include/SettingsManager.hpp" "src/SettingsManager.cpp" "src/FileWatcher.cpp" "src/FileManager.cpp" "src/MenuBar.cpp"  "src/TreeView.cpp" "src/Project.cpp")

target_include_directories(QuantomIDE PRIVATE
    src
    include
    ${PROJECT_SOURCE_DIR}/vendor/imgui
    ${PROJECT_SOURCE_DIR}/vendor/imgui/backends
    ${PROJECT_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/json/single_include
)# In your main CMakeLists.txt



target_link_libraries(QuantomIDE PRIVATE imgui glfw OpenGL::GL)

# Post-build copy runtime folder (optional)
add_custom_command(TARGET QuantomIDE POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/runtime"
    "$<TARGET_FILE_DIR:QuantomIDE>/runtime"
)
