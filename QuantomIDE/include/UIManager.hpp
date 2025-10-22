#pragma once
#define IMGUI_ENABLE_DOCKING
#include <imgui.h>
#include <vector>
#include <cstring> 



#include "FileManager.hpp"
#include "EditorManager.hpp"
#include "TreeView.hpp"
#include "MenuBar.hpp"

class UIManager {
public:
    UIManager() = default;
    ~UIManager() = default;

    void draw(EditorManager& editor,Project& e) {
        drawEditor(editor,e);
    }

    void draw(TreeView& tree,Project& p) {
        drawTreeView(tree, p);
    }


    void draw(MenuBar& menu, EditorManager& editor,Project& prj) {
        drawMenuBar(menu, editor,prj);
    }

private:
    void drawEditor(EditorManager&,Project&);
    void drawTreeView(TreeView& p_TreeView, Project& p_Project);
    void drawMenuBar(MenuBar& menuBar, EditorManager& m_Editor, Project& m_Project);
};