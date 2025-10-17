#include "Application.hpp"

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
static void ShowMainDockSpace()
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
	ImGui::Button("Build");
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
		{
			ImGuiID dock_id = ImGui::GetID("MyDockSpace");
			ImGui::SetNextWindowDockID(dock_id, ImGuiCond_FirstUseEver);
			ImGui::Begin("Editor");
			static char buffer[1024 * 16] = ""; // 16KB buffer
			ImGui::InputTextMultiline("##source", buffer, sizeof(buffer), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));

			ImGui::End();
		}

		{

			ImGuiID dock_id = ImGui::GetID("MyDockSpace");
			ImGui::SetNextWindowDockID(dock_id, ImGuiCond_FirstUseEver);
			namespace fs = std::filesystem;
			fs::path currentDir = fs::current_path();
			ImGui::Begin("File Explorer");
			for (const auto& entry : fs::directory_iterator(currentDir))
			{

				if (fs::is_directory(entry.path()))
				{
					if (ImGui::TreeNode(entry.path().filename().string().c_str()))
					{
						// You could recurse into this folder here
						ImGui::TreePop();
					}
				}
				else if (fs::is_regular_file(entry.path()))
				{
					ImGui::Text("%s", entry.path().filename().string().c_str());
					// ❌ Don't call TreePop() here, no TreeNode was opened!
				}
			}
			ImGui::End();
			ImGui::End(); // end dockspace

		}


		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New")) { /* Handle new */ }
					if (ImGui::MenuItem("Open...")) { /* Handle open */ }
					if (ImGui::MenuItem("Save")) { /* Handle save */ }
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

				ImGui::EndMainMenuBar();
			}


		}




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

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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