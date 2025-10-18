#include "UIManager.hpp"
 // For strncpy

void UIManager::draw(EditorManager& editor) {
    // For now, just draw editor UI
    drawEditor(editor);

    // You can add calls to other UI draw functions here later
}

void UIManager::drawEditor(EditorManager& editor) 
{
    auto& tabBar = editor.getTabBar();
    int tabCount = tabBar.getTabCount();

    ImGuiID dock_id = ImGui::GetID("MyDockSpace");
    ImGui::SetNextWindowDockID(dock_id, ImGuiCond_FirstUseEver);
    ImGui::Begin("Editor");

    if (tabCount == 0) {
        ImGui::Text("No files open.");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("##EditorTabs", ImGuiTabBarFlags_Reorderable)) {
        for (int i = 0; i < tabCount; ++i) {
            auto tab = tabBar.getTab(i);
            if (!tab)
                continue;

            const std::string& tabName = tab->getTabName(); // You need this getter in EditorTab

            if (ImGui::IsItemHovered()) {
                // Tab header is hovered
                ImGui::SetTooltip("File lcoation: %s", tab->getFilePath());
                // Or do other hover-related stuff here
            }
            if (ImGui::IsItemClicked())
            {

            }

            if (ImGui::BeginTabItem(tabName.c_str())) {
                auto& docText = tab->getDocument().getText();

                // Buffer size (16 KB)
                static std::vector<char> buffer(1024 * 16, 0);

                // Copy text into buffer safely
                strncpy(buffer.data(), docText.c_str(), buffer.size() - 1);
                buffer[buffer.size() - 1] = '\0';

                if (ImGui::InputTextMultiline("##source", buffer.data(), buffer.size(),
                    ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16))) {
                    tab->getDocument().setText(std::string(buffer.data()));
                }

                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
