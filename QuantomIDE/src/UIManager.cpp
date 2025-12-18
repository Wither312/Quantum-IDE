#include "imgui.h"
#include <imgui_internal.h>

#include <Core.hpp>
#include "UIManager.hpp"
#include "BuildSystem.hpp"
// For strncpy
static bool showPopup = false;
static bool projectOpen = false;
static std::string boilerPlate =
"#include <iostream>\n"
"\n"
"int main()\n"
"{\n"
"    std::cout << \"Hello world!\\n\";\n"
"    return 0;\n"
"}\n";


static inline void DrawEditorTabs(EditorManager& editor, Project& p_Project) {
	auto& tabBar = editor.getTabBar();
	int tabCount = tabBar.getTabCount();

	if (tabCount == 0) {
		ImGui::Text("No files open.");
		return;
	}
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard; // temporarily disable keyboard navigation

	if (ImGui::BeginTabBar("##EditorTabs", ImGuiTabBarFlags_Reorderable | ImGuiInputTextFlags_CallbackCharFilter)) {
		for (int i = 0; i < tabCount; ++i) {
			auto tab = tabBar.getTab(i);
			if (!tab)
				continue;

			std::string tabLabel = tab->getTabName(); // Visible part
			if (tab->getDocument().isDirty())
				tabLabel += "*";

			// Append unique ID (invisible) to avoid ImGui ID collisions
			tabLabel += "##" + tab->getID();

			if (ImGui::BeginTabItem(tabLabel.c_str())) {
				auto& docText = tab->getDocument().getText();

				static std::vector<char> buffer(1024 * 16, 0);
				strncpy(buffer.data(), docText.c_str(), buffer.size() - 1);
				buffer[buffer.size() - 1] = '\0';

				if (tab->getFocusEditorNextFrame()) {
					ImGui::SetKeyboardFocusHere();
					tab->setFocusEditorNextFrame(false);
				}

				// Fill the remaining vertical space in the Editor window
				ImVec2 availableSpace = ImGui::GetContentRegionAvail();

				
				ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackAlways;

				std::string editorLabel = "##editor" + tab->getID();

				if (ImGui::InputTextMultiline(editorLabel.c_str(), buffer.data(), buffer.size(),
					availableSpace, flags,
					[](ImGuiInputTextCallbackData* data) -> int {
						Document* doc = static_cast<Document*>(data->UserData);
						if (!doc) return 0;
						ImGuiIO& io = ImGui::GetIO();
						if (ImGui::IsKeyPressed(ImGuiKey_LeftBracket) && io.KeyShift)
						{
							const char ch = '}'; // character to insert
							size_t pos = data->CursorPos;

							// Shift buffer to make space for the char
							if (data->BufTextLen + 1 < data->BufSize)
							{
								memmove(data->Buf + pos + 1, data->Buf + pos, strlen(data->Buf + pos) + 1);
								data->Buf[pos] = ch;
								data->CursorPos++;
								data->BufTextLen++;
								data->BufDirty = true;

								// Optionally update your Document
								Document* doc = static_cast<Document*>(data->UserData);
								if (doc) doc->setText(std::string(data->Buf));

								data->CursorPos -= 1;

								return 1; // handled
							}
						}
						if (ImGui::IsKeyPressed(ImGuiKey_9) && io.KeyShift)
						{
							const char ch = ')'; // character to insert
							size_t pos = data->CursorPos;

							// Shift buffer to make space for the char
							if (data->BufTextLen + 1 < data->BufSize)
							{
								memmove(data->Buf + pos + 1, data->Buf + pos, strlen(data->Buf + pos) + 1);
								data->Buf[pos] = ch;
								data->CursorPos++;
								data->BufTextLen++;
								data->BufDirty = true;

								// Optionally update your Document
								Document* doc = static_cast<Document*>(data->UserData);
								if (doc) doc->setText(std::string(data->Buf));

								data->CursorPos -= 1;

								return 1; // handled
							}
						}
						// Check for TAB key pressed
						if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways &&
							ImGui::IsKeyPressed(ImGuiKey_Tab))
						{
							const char tab[] = "    "; // 4 spaces
							size_t pos = data->CursorPos;

							// Insert tab into buffer
							data->BufTextLen += 4; // increase text length
							data->BufDirty = true;

							// Shift buffer contents to make space for tab
							if (data->BufTextLen < data->BufSize)
							{
								memmove(data->Buf + pos + 4, data->Buf + pos, strlen(data->Buf + pos) + 1);
								memcpy(data->Buf + pos, tab, 4);
								data->CursorPos += 4;
							}

							// Update Document
							doc->setText(std::string(data->Buf));
							doc->setCursorPos(data->CursorPos);

							return 1; // event handled
						}

						// Always sync cursor position
						doc->setCursorPos(data->CursorPos);

						return 0;
					}, &tab->getDocument()))
				{
					// If text changed normally (not TAB)
					tab->getDocument().setText(std::string(buffer.data()));
				}

				


				ImGui::EndTabItem();
			}

			if (ImGui::IsItemHovered()) {
				if (!tab->getFilePath().empty()) {
					std::string filePath = tab->getFilePath().string();
					ImGui::SetTooltip("File location: %s", filePath.c_str());
				}
			}

			if (ImGui::IsItemClicked()) {
				editor.getTabBar().setCurrentTabIndex(i);
				LOG("%s", core::Log::LogLevel::Tracer, ("Tab " + std::to_string(i) + " has been clicked!").c_str());
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
				editor.getTabBar().closeTab(i);
				std::cout << "Tab " << i << " (MMB) is clicked!\n";
			}

			if (ImGui::BeginPopupContextItem("customID_1")) {
				if (ImGui::MenuItem("Add file to project")) {
					if (p_Project.isOpen()) {
						p_Project.addSourceFile(editor.getTabBar().getCurrentTab()->getFilePath());
					}
					else
					{
						std::cout << "No project is open\n";
					}
				}
				if (ImGui::MenuItem("Compile")) {
					// Handle compile
				}
				ImGui::EndPopup();
			}
		}
		ImGui::EndTabBar();
	}
}


static void DrawEditorDockspace(EditorManager& editor, Project& p_Project) {
	ImGuiID dockspace_id = ImGui::GetID("MainEditorDockspace");

	ImVec2 dockspace_size = ImGui::GetContentRegionAvail();
	ImGui::DockSpace(dockspace_id, dockspace_size, ImGuiDockNodeFlags_None);

	static bool initialized = false;
	if (!initialized) {
		initialized = true;

		ImGui::DockBuilderRemoveNode(dockspace_id);
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

		// Split into top (editor), bottom (console/output)
		ImGuiID dock_main = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.7f, nullptr, &dockspace_id);
		ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.3f, nullptr, &dockspace_id);

		ImGui::DockBuilderDockWindow("Editor", dock_main);
		ImGui::DockBuilderDockWindow("Console", dock_bottom);
		ImGui::DockBuilderDockWindow("Output", dock_bottom);

		ImGui::DockBuilderFinish(dockspace_id);
	}


	// Console window
	ImGui::Begin("Console");

	ImGui::Text("Console output here...");
	ImGui::End();

	// Output window
	ImGui::Begin("Output");
	ImGui::Text("Build output here...");
	ImGui::End();

	// Editor window
	ImGui::Begin("Editor");
	DrawEditorTabs(editor, p_Project);
	ImGui::End();
}

void UIManager::drawEditor(EditorManager& editor, Project& p_Project) {
	ImGuiID rootDockspaceID = ImGui::GetID("MyDockSpace"); // Must match Application::ShowMainDockSpace()

	static bool initialized = false;
	if (!initialized) {
		initialized = true;

		// Clean up any existing layout
		ImGui::DockBuilderRemoveNode(rootDockspaceID);
		ImGui::DockBuilderAddNode(rootDockspaceID, ImGuiDockNodeFlags_DockSpace);

		// Set size (usually viewport size for fullscreen)
		ImGui::DockBuilderSetNodeSize(rootDockspaceID, ImGui::GetMainViewport()->WorkSize);

		// 1. Split into LEFT and RIGHT (left = 16% width)
		ImGuiID dock_left = 0;
		ImGuiID dock_right = 0;
		ImGui::DockBuilderSplitNode(rootDockspaceID, ImGuiDir_Left, 0.16f, &dock_left, &dock_right);

		ImGuiID dock_main = 0;
		ImGuiID dock_status = 0;
		ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.07f, &dock_status, &dock_main);

		// 2. Dock File Explorer and Project into LEFT dock (tabbed together)
		ImGui::DockBuilderDockWindow("File Explorer", dock_left);
		ImGui::DockBuilderDockWindow("Project", dock_left);

		// 3. Dock Editor, Console, Output all into RIGHT dock (tabbed together)
		ImGui::DockBuilderDockWindow("Editor", dock_main);
		ImGui::DockBuilderDockWindow("Console", dock_main);
		ImGui::DockBuilderDockWindow("Output", dock_main);
		ImGui::DockBuilderDockWindow("Status", dock_status);

		ImGui::DockBuilderFinish(rootDockspaceID);
	}

	// 🧱 Create docked windows
	ImGui::Begin("Editor");
	DrawEditorTabs(editor, p_Project);
	ImGui::End();

	ImGui::Begin("Console"); {
		// Option 1: Read-only multiline with wrapping (better for logs)
		ImGui::TextWrapped("%s", BuildSystem::s_ConsoleOutput.c_str());

		// Option 2: Editable multiline text box (if you want it editable)
		static std::string buffer;
		buffer = BuildSystem::s_ConsoleOutput; // update buffer with latest output

		// Create a char buffer (InputTextMultiline needs char*), static so it persists
		static std::vector<char> charBuffer(1024 * 16, 0);
		strncpy(charBuffer.data(), buffer.c_str(), charBuffer.size() - 1);
		charBuffer[charBuffer.size() - 1] = '\0';

		ImGui::End();
	}

	ImGui::Begin("Output");

	// Option 1: Read-only multiline with wrapping (better for logs)
	ImGui::TextWrapped("%s", BuildSystem::s_BuildOutput.c_str());

	// Option 2: Editable multiline text box (if you want it editable)
	static std::string buffer;
	buffer = BuildSystem::s_BuildOutput; // update buffer with latest output

	// Create a char buffer (InputTextMultiline needs char*), static so it persists
	static std::vector<char> charBuffer(1024 * 16, 0);
	strncpy(charBuffer.data(), buffer.c_str(), charBuffer.size() - 1);
	charBuffer[charBuffer.size() - 1] = '\0';


	ImGui::End();


	ImGui::Begin("File Explorer");
	ImGui::End();

	ImGui::Begin("Project");
	ImGui::End();
}


void UIManager::drawTreeView(TreeView& p_TreeView, Project& p_Project) {
	ImGuiID dock_id = ImGui::GetID("MyDockSpace");
	ImGui::SetNextWindowDockID(dock_id, ImGuiCond_FirstUseEver);

	namespace fs = std::filesystem;
	fs::path currentDir = fs::current_path();

	ImGui::Begin("File Explorer");

	// Recursive helper function defined inside the method
	std::function<void(const fs::path&)> drawDirectory = [&](const fs::path& dirPath) {
		for (const auto& entry : fs::directory_iterator(dirPath)) {
			if (entry.is_directory()) {
				if (ImGui::TreeNode(entry.path().filename().string().c_str())) {
					drawDirectory(entry.path()); // recursive call
					ImGui::TreePop();
				}
			}
			else if (entry.is_regular_file()) {
				const std::string filename = entry.path().filename().string();

				if (ImGui::Selectable(filename.c_str())) {
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

	std::unordered_map<std::string, std::vector<std::filesystem::path> > filesByFilter;
	for (const auto& file : sourceFiles) {
		auto it = fileToFilter.find(file);
		std::string filter = (it != fileToFilter.end()) ? it->second : "Uncategorized";
		filesByFilter[filter].push_back(file);
	}

	ImGui::SetNextWindowDockID(dock_id, ImGuiCond_FirstUseEver);
	ImGui::Begin("Project");

	for (const auto& filterName : { "sourceFiles", "Source Files", "Resource Files" }) {
		ImGui::PushID(filterName);

		bool open = ImGui::TreeNodeEx(filterName, ImGuiTreeNodeFlags_DefaultOpen);

		ImGui::SameLine();


		if (ImGui::Selectable("+", false, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_SpanAllColumns)) {
			if (!(p_Project.isOpen())) {
				projectOpen = true;
			}
			else {
				extern core::Core g_Core;

				std::filesystem::path selected = g_Core.getFileSystem()->openFile().value();
				if (!selected.empty()) {
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

		if (open) {
			const std::vector<std::filesystem::path>& files = filesByFilter[filterName];
			for (const auto& file : files) {
				std::string filename = file.filename().string();
				if (ImGui::Selectable(filename.c_str())) {
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

void UIManager::drawMenuBar(MenuBar& menuBar, EditorManager& p_Editor, Project& p_Project) {
	extern core::Core g_Core;

	if (ImGui::BeginMainMenuBar()) {
		// FILE MENU
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				p_Editor.getTabBar().addTab(std::make_unique<EditorTab>(
					std::string("Not saved " + std::to_string(p_Editor.getTabBar().getTabCount()))));
				p_Editor.getTabBar().setCurrentTabIndex(p_Editor.getTabBar().getTabCount() - 1);
			}

			if (ImGui::MenuItem("Open")) {
				auto selectedFile = g_Core.getFileSystem()->openFile(
					"Text Files\0*.txt\0C++ Files\0*.cpp;*.h\0All Files\0*.*\0");
				if (selectedFile.has_value()) {
					std::fstream fileData(selectedFile.value(), std::ios::in);
					std::string buffer;
					fileData >> buffer;

					if (!selectedFile.value().empty()) {
						p_Editor.openFile(selectedFile.value().string());
					}
				}
				else {
					LOG("Warning: File open operation failed or was cancelled by user.", core::Log::LogLevel::Warn);
				}
			}

			if (ImGui::MenuItem("Save")) {
				const auto& tab = p_Editor.getTabBar().getCurrentTab();

				if (!tab) {
					std::cerr << "[Warning] Cannot save: No tab is open.\n";
				}
				else {
					auto path = tab->save();
					if (path.has_value()) {
						tab->setFilePath(path.value());
						tab->getDocument().markClean();
					}
					else {
						LOG("Warning: File failed to save.", core::Log::LogLevel::Warn);
					}
				}
			}

			if (ImGui::MenuItem("Save As")) {
				const auto& tab = p_Editor.getTabBar().getCurrentTab();
				auto filePath = g_Core.getFileSystem()->saveFile(tab->getDocument().getText()).value();

				if (!filePath.empty()) {
					tab->setFilePath(filePath);
					tab->setTabName(std::filesystem::path(filePath).filename().string());
					g_Core.getFileSystem()->saveFile(tab->getDocument().getText(), tab->getFilePath());
				}
			}

			ImGui::EndMenu(); // <-- closes File menu properly
		}

		// EDIT MENU
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo")) {
			}
			if (ImGui::MenuItem("Redo")) {
			}
			ImGui::EndMenu();
		}

		// VIEW MENU
		if (ImGui::BeginMenu("View")) {
			//	ImGui::MenuItem("Show Console", NULL, &show_console_window);
			//	ImGui::MenuItem("Show Inspector", NULL, &show_inspector_window);
			ImGui::EndMenu();
		}

		// PROJECT MENU
		if (ImGui::BeginMenu("Project")) {
			if (ImGui::MenuItem("New")) {
				showPopup = true; // will trigger popup next frame
			}

			if (ImGui::MenuItem("Open")) {
				auto filePath = g_Core.getFileSystem()->openFile("Project Files\0*.qprj\0All Files\0*.*\0");

				if (filePath.has_value()) {
					if (p_Project.open(filePath.value())) {
						p_Editor.getTabBar().closeAll();
						for (auto& file : p_Project.getSourceFiles())
							p_Editor.openFile(file.string());
					}
					else {
						ImGui::OpenPopup("Error##ProjectOpen");
					}
				}
				else {
					LOG("Warning: Project Open operation failed or was cancelled by user.", core::Log::LogLevel::Warn);
				}
			}

			if (ImGui::MenuItem("Save")) {
				p_Project.save();
			}

			if (ImGui::MenuItem("Open Folder")) {
				std::filesystem::path path = p_Project.getRootDirectory();
				if (std::filesystem::exists(path)) {
					std::string command = "explorer \"" + path.string() + "\"";
					system(command.c_str());
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (projectOpen) {
		ImGui::OpenPopup("Open a project to create a file");
		projectOpen = false;
	}

	if (ImGui::BeginPopupModal("Open a project to create a file", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("No project is opened.");
		if (ImGui::Button("OK"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	if (showPopup) {
		ImGui::OpenPopup("New Project");
		showPopup = false;
	}

	if (ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		static char projectName[128] = "";

		ImGui::InputText("Project Name", projectName, IM_ARRAYSIZE(projectName));

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			std::filesystem::path basePath = g_Core.getFileSystem()->openFolder().value();
			if (!basePath.empty()) {
				std::filesystem::path projectDir = basePath / projectName;
				std::filesystem::create_directory(projectDir);
				p_Editor.getTabBar().closeAll();
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
			projectName[0] = '\0'; // Reset input
		}


		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			projectName[0] = '\0'; // Reset input
		}

		ImGui::EndPopup();
	}
}

void UIManager::drawStatusBar(StatusBar& statusBar, EditorManager& editor, Project& m_Project) {
	ImGui::Begin("Status");
	auto* tab = editor.getTabBar().getCurrentTab();
	if (tab) {
		auto [line, col] = tab->getDocument().getCursorPos();
		ImGui::Text("Ln %d, Col %d | %s", static_cast<int>(line) + 1, static_cast<int>(col) + 1, tab->getTabName().c_str());
	}
	else {
		ImGui::Text("No file open.");
	}
	ImGui::End();
}
