#pragma once

#include <string>
#include <memory>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <iostream>
#ifdef WIN32
#include <Windows.h>
#include <commdlg.h>
#endif
#include <fstream>


#include "EditorManager.hpp"
#include "UIManager.hpp"
#include "BuildSystem.hpp"

// Forward declarations to avoid unnecessary includes
struct GLFWwindow;

// Application class managing the core lifecycle
class Application
{
public:
    // Constructor / Destructor
    Application(const std::string& title = "Quantom IDE", int width = 1280, int height = 720);
    ~Application();

    // Prevent copy
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Allow move (optional)
    Application(Application&&) noexcept = default;
    Application& operator=(Application&&) noexcept = default;

    // Main run loop
    void Run();

    // Should be called once per frame (used for testing or external integration)
    void Update();

    // Accessors
    inline GLFWwindow* GetWindow() const { return m_Window; }
    inline bool ShouldClose() const;

private:
    // Internal methods
    void InitGLFW();
    void InitImGui();
    void Shutdown();

    void ShowMainDockSpace();

    void BeginFrame();
    void EndFrame();

private:
    GLFWwindow* m_Window = nullptr;
    ImGuiIO* m_IO = nullptr;
    std::string m_Title;
    int m_Width;
    int m_Height;
    bool m_Running = false;
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    EditorManager m_Editor;
    UIManager m_UIManager;
    BuildSystem m_BuildSytem;

};
