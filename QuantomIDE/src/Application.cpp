#include "Application.hpp"
#include "Core.hpp"
#include <imgui_internal.h>
#define IMGUI_ENABLE_DOCKING

core::Core g_Core;
LSPClient g_LSPClient("C:\\Program Files\\LLVM\\bin\\clangd.exe", { "--log=verbose", "--all-scopes-completion", "--background-index", "--completion-style=detailed" });
//LSPClient g_LSPClient("clangd", { "--log=verbose", "--all-scopes-completion", "--background-index", "--completion-style=detailed" });

inline static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

Application::Application(const std::string& title, int width, int height)
	: m_Title(title), m_Width(width), m_Height(height)
{
	InitGLFW();
	InitImGui();

	//Window options
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;


	if (!g_Core.initialize())
	{
		std::cerr << "Core not init!\n";
	}

	if (!g_LSPClient.start()) {
		LOG("[LSP Client] clangd failed to start!", core::Log::LogLevel::Error);
	}

	g_LSPClient.setOnCompletion([this](int id, const std::vector<CompletionItem>& items) {
        LOG("Received %zu completion items for ID %d", core::Log::Tracer, items.size(), id);
        for (const auto& item : items) {
            LOG(" - %s (%s)", core::Log::Tracer, item.label.c_str(), item.detail.empty() ? "" : item.detail.c_str());
        }
        m_completionItems = items;
        m_pendingCompletionId = id;
    });
}
Application::~Application()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}
void Application::Run()
{
	// Initialize ImGui after window creation

	Update();  // Run the main loop

	Shutdown();
}
enum class IDEStatus {
	Ready,
	Building,
	Debugging,
	Error,
	Warning
};

IDEStatus currentIDEStatus = IDEStatus::Ready;



void Application::ShowMainDockSpace()
{
	static bool dockspaceOpen = true;
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// Main dockspace window
	ImGui::Begin("MainDockSpace", &dockspaceOpen, window_flags);

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);



	ImGuiWindowFlags toolbar_flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoDocking;


	float toolbar_height = 40.0f;
	ImVec2 toolbar_pos = ImGui::GetCursorScreenPos();
	ImVec2 toolbar_size = ImVec2(ImGui::GetContentRegionAvail().x, toolbar_height);

	ImGui::BeginChild("Toolbar", toolbar_size, false, toolbar_flags);
	// Your toolbar buttons
	ImGui::Button("New");
	ImGui::SameLine();
	ImGui::Button("Save");
	ImGui::SameLine();
	if (ImGui::Button("Build")) {
		// Handle build action
		m_BuildSytem.BuildCurrentProject(m_Editor, m_Project);
	}
	ImGui::SameLine();
	if (ImGui::Button("Run")) {
		// Handle build action
		m_BuildSytem.RunCurrentProject(m_Project);
	}
	ImGui::SameLine();
	if (ImGui::Button("Build and Run"))
	{
		// Handle build action
		m_BuildSytem.BuildCurrentProject(m_Editor, m_Project);
		m_BuildSytem.RunCurrentProject(m_Project);
	}
	ImGui::SameLine();
	if (ImGui::Button("Debug"))
	{
		if (!m_debugSessionActive) m_debugSessionActive = true;
		m_DebugSystem = DebugSystem();
		if (!m_DebugSystem.loadExecutable(m_Project.getRootDirectory().string() + "\\" + m_Project.getName()))
		{
			m_DebugSystem.addBreakpoint(m_Project.getName() + ".cpp", 5);
			m_DebugSystem.getLocalVariables();
			m_DebugSystem.run();
		}


	}
	if (m_debugSessionActive)
	{
		ImGui::SameLine();
		ImGui::Text("|");
		ImGui::SameLine();
		if (ImGui::Button("Stop Debug"))
		{
			if (m_debugSessionActive) m_debugSessionActive = false;
			m_DebugSystem.stop();

		}
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
		{
			// Handle build action
			//TODO ADD PAUSE m_DebugSystem.isPaused();

		}
		ImGui::SameLine();
		if (ImGui::Button("Step Over"))
		{
			// Handle build action
			m_DebugSystem.step();
		}
		ImGui::SameLine();
		if (ImGui::Button("Step Into"))
		{
			// Handle build action
			m_DebugSystem.next();


		}
		ImGui::SameLine();
		if (ImGui::Button("Step Out"))
		{
			// Handle build action


		}


	}
	// Add more buttons as needed...

	ImGui::EndChild();



	// DockSpace ID
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

}


void Application::Update()
{
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();
		if (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}


		this->BeginFrame();

		ShowMainDockSpace();

		bool ctrlPressed = m_IO->KeyCtrl;  // or io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
		if (ctrlPressed && ImGui::IsKeyPressed(ImGuiKey_S)) {
			// Save action here
			m_Editor.getTabBar().getCurrentTab()->save();
			std::cout << m_Editor.getTabBar().getCurrentTab()->getDocument().getCursorPos().first << " " << m_Editor.getTabBar().getCurrentTab()->getDocument().getCursorPos().second << std::endl;
		}

		// Ctrl+Space - Trigger completion
        if (ctrlPressed && ImGui::IsKeyPressed(ImGuiKey_Space)) {
            if (EditorTab* tab = m_Editor.getTabBar().getCurrentTab()) {
                auto cursorPos = tab->getDocument().getCursorPos();
                m_pendingCompletionId = g_LSPClient.textDocumentCompletion(
                    tab->getFilePath(), 
                    cursorPos.first + 1, cursorPos.second + 1
                );
                LOG("Sent completion request ID: %d at (%d,%d)", core::Log::Tracer, 
                    m_pendingCompletionId, cursorPos.first + 1, cursorPos.second + 1);
                m_showCompletionPopup = true;  // Show loading state
            }
        }

        // === COMPLETION POPUP ===
        if (m_showCompletionPopup) {
            ImGui::OpenPopup("Completions");
            if (ImGui::BeginPopup("Completions", ImGuiWindowFlags_AlwaysAutoResize)) {
                if (m_completionItems.empty()) {
                    ImGui::Text("Loading completions...");
                } else {
                    ImGui::Text("Completions (%zu)", m_completionItems.size());
                    ImGui::Separator();
                    
                    for (size_t i = 0; i < m_completionItems.size(); ++i) {
                        const auto& item = m_completionItems[i];
                        std::string display = item.label;
                        if (!item.detail.empty()) {
                            display += " ##detail_" + std::to_string(i);
                        }
                        
                        if (ImGui::Selectable(display.c_str())) {
                            // Insert selected completion
                            m_Editor.insertText(item.insertText);
                            m_completionItems.clear();
                            m_pendingCompletionId = -1;
                            m_showCompletionPopup = false;
                            ImGui::CloseCurrentPopup();
                            break;
                        }
                        
                        if (ImGui::IsItemHovered() && !item.detail.empty()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("%s", item.detail.c_str());
                            ImGui::EndTooltip();
                        }
                    }
                }
                ImGui::EndPopup();
            }
        }

		//TOOD find better way to index 
		m_UIManager.draw(m_Editor, m_Project);

		m_UIManager.draw(m_TreeView, m_Project);

		m_UIManager.draw(m_MenuBar, m_Editor, m_Project);

		m_UIManager.draw(m_StatusBar, m_Editor, m_Project);


		this->EndFrame();
	}
}
inline bool Application::ShouldClose() const
{
	return false;
}
void Application::InitGLFW()
{
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit())
	{
		// Handle initialization failure (throw or set error state)
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
	if (!m_Window)
	{
		glfwTerminate();
		// Handle window creation failure
		return;
	}

	glfwMakeContextCurrent(m_Window);
	glfwSwapInterval(1); // Enable vsync
}
void Application::InitImGui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_IO = &ImGui::GetIO();  // Assign m_IO properly here
	m_IO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	m_IO->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	m_IO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	m_IO->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	ImGui::StyleColorsDark();

	float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
	const char* glsl_version = "#version 130";

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 3
	m_IO->ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
	m_IO->ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.
#endif

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	if (m_IO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
#ifdef __EMSCRIPTEN__
	ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
	ImGui_ImplOpenGL3_Init(glsl_version);


}
void Application::Shutdown()
{

}

void Application::BeginFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}
void Application::EndFrame()
{


	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(m_Window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (m_IO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
	glfwSwapBuffers(m_Window);

}