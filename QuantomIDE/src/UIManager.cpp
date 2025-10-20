#include "UIManager.hpp"
// For strncpy
static bool showPopup = false;
static bool projectOpen = false;
static std::string boilerPlate =
"#include <iostream>\n"
"\n"
"int main()\n"
"{\n"
"    std::cout << \"hello world\\n\";\n"
"    return 0;\n"
"}\n";



#include "imgui.h"
#include <imgui_internal.h>

static void DrawEditorDockspaceWithConsoleAndOutput()
{
	// Create internal dockspace
	ImGuiID editor_dock_id = ImGui::GetID("EditorInnerDockspace");
	ImVec2 dockspace_size = ImGui::GetContentRegionAvail();
	ImGui::DockSpace(editor_dock_id, dockspace_size, ImGuiDockNodeFlags_None);

	// Setup layout once
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;

		ImGui::DockBuilderRemoveNode(editor_dock_id); // Clear any previous layout
		ImGui::DockBuilderAddNode(editor_dock_id, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(editor_dock_id, dockspace_size);

		// Split top (main editor area) and bottom (for tabs)
		ImGuiID dock_main = ImGui::DockBuilderSplitNode(editor_dock_id, ImGuiDir_Up, 0.6f, nullptr, &editor_dock_id);
		ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(editor_dock_id, ImGuiDir_Down, 0.4f, nullptr, &editor_dock_id);

		// Tab both in bottom
		ImGui::DockBuilderDockWindow("Console", dock_bottom);
		ImGui::DockBuilderDockWindow("Output", dock_bottom);

		ImGui::DockBuilderFinish(editor_dock_id);
	}

	// End Editor window *after* building dockspace and drawing tabbed windows

	// Begin/End Console window
	ImGui::Begin("Console");
	ImGui::Text("Console output here...");
	ImGui::End();

	// Begin/End Output window
	ImGui::Begin("Output");
	ImGui::Text("Build output here...");
	ImGui::End();
}


void UIManager::drawEditor(EditorManager& editor,Project& p_Project)
{
	auto& tabBar = editor.getTabBar();
	int tabCount = tabBar.getTabCount();

	ImGuiID dock_id = ImGui::GetID("MyDockSpace");
	ImGui::SetNextWindowDockID(dock_id, ImGuiCond_FirstUseEver);
	ImGui::Begin("Editor");
	DrawEditorDockspaceWithConsoleAndOutput();
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
			if (ImGui::IsItemHovered()) {
				// Tab header is hovered
				if (tab->getFilePath() != "")
				{
					std::string filePath = tab->getFilePath().string();
					ImGui::SetTooltip("File lcoation: %s", filePath.c_str());
				}
				// Or do other hover-related stuff here
			}
			if (ImGui::IsItemClicked())
			{
				//get the position of the item in the vector and set current tab to that -1 because 0 is first element
				editor.getTabBar().setCurrentTabIndex(i);
				std::cout << "Tab " << i << " is clicked!\n";
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Middle))
			{
				editor.getTabBar().closeTab(i);
				std::cout << "Tab " << i << " (MMB) is clicked!\n";
			}
			static bool contextMenu = false;
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				contextMenu = true;
			}
			if (ImGui::BeginPopupContextItem()) // This opens on right-click
			{
				if (ImGui::MenuItem("Add file to project"))
				{
					if (p_Project.isOpen())
					{
						p_Project.addSourceFile(editor.getTabBar().getCurrentTab()->getFilePath());
					}
					else
					{
						projectOpen = true;
					}
					contextMenu = false;
				}
				if (ImGui::MenuItem("Compile"))
				{
					contextMenu = false; 
				}
				ImGui::EndPopup();
			}
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}



void UIManager::drawTreeView(TreeView& p_TreeView, Project& p_Project)
{
	ImGuiID dock_id = ImGui::GetID("MyDockSpace");
	ImGui::SetNextWindowDockID(dock_id, ImGuiCond_FirstUseEver);

	namespace fs = std::filesystem;
	fs::path currentDir = fs::current_path();

	ImGui::Begin("File Explorer");

	// Recursive helper function defined inside the method
	std::function<void(const fs::path&)> drawDirectory = [&](const fs::path& dirPath) {
		for (const auto& entry : fs::directory_iterator(dirPath))
		{
			if (entry.is_directory())
			{
				if (ImGui::TreeNode(entry.path().filename().string().c_str()))
				{
					drawDirectory(entry.path());  // recursive call
					ImGui::TreePop();
				}
			}
			else if (entry.is_regular_file())
			{
				const std::string filename = entry.path().filename().string();

				if (ImGui::Selectable(filename.c_str()))
				{
					// File was clicked — call your load file function here
					//loadFile(entry.path());
					//TODO load file into editor
				}
			}
		}
		};

	drawDirectory(currentDir);

	ImGui::End();

	auto rootDir = p_Project.getRootDirectory();
	const auto& sourceFiles = p_Project.getSourceFiles(); // relative paths
	const auto& fileToFilter = p_Project.getFileFilters(); // path -> filter

	std::unordered_map<std::string, std::vector<std::filesystem::path>> filesByFilter;
	for (const auto& file : sourceFiles)
	{
		auto it = fileToFilter.find(file);
		std::string filter = (it != fileToFilter.end()) ? it->second : "Uncategorized";
		filesByFilter[filter].push_back(file);
	}

	ImGui::SetNextWindowDockID(dock_id, ImGuiCond_FirstUseEver);
	ImGui::Begin("Project");

	for (const auto& filterName : { "sourceFiles", "Source Files", "Resource Files" })
	{
		ImGui::PushID(filterName);

		bool open = ImGui::TreeNodeEx(filterName, ImGuiTreeNodeFlags_DefaultOpen);

		ImGui::SameLine();


		if (ImGui::Selectable("+", false, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_SpanAllColumns))
		{
			if (!(p_Project.isOpen()))
			{
				projectOpen = true;
			}
			else
			{

				std::filesystem::path selected = FileDialog::openFileDialog();
				if (!selected.empty())
				{
					auto relPath = std::filesystem::relative(selected, rootDir);

					if (filterName == std::string("Header Files"))
						p_Project.addHeaderFile(selected);
					else if (filterName == std::string("Source Files"))
						p_Project.addSourceFile(selected);
					else if (filterName == std::string("Resource Files"))
						p_Project.addResourceFile(selected);
				}
			}
		}

		if (open)
		{
			const std::vector<std::filesystem::path>& files = filesByFilter[filterName];
			for (const auto& file : files)
			{
				std::string filename = file.filename().string();
				if (ImGui::Selectable(filename.c_str()))
				{
					auto absPath = rootDir / file;
					// loadFile(absPath);
				}
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	ImGui::End();






	ImGui::End(); // end dockspace

}

void UIManager::drawMenuBar(MenuBar& menuBar, EditorManager& p_Editor, Project& p_Project)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
				p_Editor.getTabBar().addTab(std::make_unique<EditorTab>(std::string("Not saved " + std::to_string(p_Editor.getTabBar().getTabCount()))));
				p_Editor.getTabBar().setCurrentTabIndex(p_Editor.getTabBar().getTabCount() - 1);
			}
			if (ImGui::MenuItem("Open..."))
			{
#ifdef WIN32
				std::string selectedFile = FileDialog::openFileDialog("Text Files\0*.txt\0C++ Files\0*.cpp;*.h\0All Files\0*.*\0");
				if (!selectedFile.empty()) {
					// Handle file open, e.g. load file contents into your editor
					p_Editor.openFile(selectedFile);

				}

#endif
			}
			if (ImGui::MenuItem("Save")) {
				const auto& tab = p_Editor.getTabBar().getCurrentTab();

				if (!tab) {
					std::cerr << "[Warning] Cannot save: No tab is open.\n";
				}
				else {
					if (tab->getFilePath().empty()) {
						std::string filePath = FileDialog::saveFileDialog();
						if (!filePath.empty()) { // user selected a path
							tab->setFilePath(filePath);
							tab->setTabName(std::filesystem::path(filePath).filename().string());
							FileManager::saveFile(filePath, tab->getDocument().getText());
						}
					}
					else {
						FileManager::saveFile(tab->getFilePath(), tab->getDocument().getText());
					}
				}
			}

			if (ImGui::MenuItem("Save As")) {
				const auto& tab = p_Editor.getTabBar().getCurrentTab();
				std::string filePath = FileDialog::saveFileDialog();
				if (!filePath.empty()) { // always good to check for cancel
					tab->setFilePath(filePath);
					tab->setTabName(std::filesystem::path(filePath).filename().string());
					FileManager::saveFile(tab->getFilePath(), tab->getDocument().getText());
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo")) {}
			if (ImGui::MenuItem("Redo")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			//	ImGui::MenuItem("Show Console", NULL, &show_console_window);
			//	ImGui::MenuItem("Show Inspector", NULL, &show_inspector_window);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Project"))
		{

			if (ImGui::MenuItem("New"))
			{
				showPopup = true;  // will trigger popup next frame
			}



			if (ImGui::MenuItem("Open"))
			{
				// Open file dialog to pick a project file (e.g. .qprj or similar)
				std::string filePath = FileDialog::openFileDialog("Project Files\0*.qprj\0All Files\0*.*\0");
				if (!filePath.empty())
				{
					if (p_Project.open(filePath))
					{
						for (auto& file : p_Project.getSourceFiles())
						{
							p_Editor.openFile(file.string());

						}
						

						// Show error popup or message
					}
					else
					{

						ImGui::OpenPopup("Error##ProjectOpen");
					}
				}
			}

			if (ImGui::MenuItem("Save"))
			{

				if (!p_Project.save())
				{
					// Show error popup or message
					ImGui::OpenPopup("Error##ProjectOpen");
				}

			}

			if (ImGui::MenuItem("Open Folder"))
			{
				std::filesystem::path path = p_Project.getRootDirectory();
				if (std::filesystem::exists(path))
				{
					std::string command = "explorer \"" + path.string() + "\"";
					system(command.c_str());
				}
			}

			ImGui::EndMenu();
		}



		ImGui::EndMainMenuBar();
	}
	if (projectOpen)
	{
		ImGui::OpenPopup("Open a project to create a file");
		projectOpen = false;
	}

	if (ImGui::BeginPopupModal("Open a project to create a file", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("No project is opened.");
		if (ImGui::Button("OK"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	if (showPopup)
	{
		ImGui::OpenPopup("New Project");
		showPopup = false;
	}

	if (ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char projectName[128] = "";

		ImGui::InputText("Project Name", projectName, IM_ARRAYSIZE(projectName));

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			std::filesystem::path basePath = FileDialog::openFolderDialog();
			if (!basePath.empty()) {
				std::filesystem::path projectDir = basePath / projectName;
				std::filesystem::create_directory(projectDir);

				p_Project = Project{}; // Clear current
				p_Project.createNew(projectDir, projectName);
				p_Project.save();

				// Create main.cpp inside project directory
				std::filesystem::path mainFilePath = projectDir / (std::string(projectName) + ".cpp");
				std::ofstream main(mainFilePath);
				main << boilerPlate;
				main.close();

				p_Project.addSourceFile(mainFilePath);
				p_Editor.openFile(mainFilePath.string());
			}

			ImGui::CloseCurrentPopup();
			projectName[0] = '\0';  // Reset input
		}


		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			projectName[0] = '\0';  // Reset input
		}

		ImGui::EndPopup();
	}


}

