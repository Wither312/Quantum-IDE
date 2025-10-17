#pragma once
#include <imgui.h>
#include <vector>
#include <cstring> 
#include "EditorManager.hpp"

class UIManager {
public:
    UIManager() = default;
    ~UIManager() = default;

    // Main UI draw call, takes EditorManager reference
    void draw(EditorManager& editor);

private:
    // Separate method to draw editor UI
    void drawEditor(EditorManager& editor);

    // You can add more UI components here later
    // e.g., void drawMenuBar();
};