#include "Application.hpp"
#include <Core.hpp>
#include <imgui_internal.h>
#define IMGUI_ENABLE_DOCKING

core::Core g_Core;

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
		
		
	}
	if(m_debugSessionActive)
	{
		ImGui::SameLine();
		ImGui::Text("|");
		ImGui::SameLine();
		if (ImGui::Button("Stop Debug"))
		{
			if (m_debugSessionActive) m_debugSessionActive = false;
			
		}
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
		{
			// Handle build action
			
			
		}
		ImGui::SameLine();
		if (ImGui::Button("Step Over"))
		{
			// Handle build action
			
			
		}
		ImGui::SameLine();
		if (ImGui::Button("Step Into"))
		{
			// Handle build action
			
			
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
		}

		//TOOD find better way to index 
		m_UIManager.draw(m_Editor, m_Project);

		m_UIManager.draw(m_TreeView, m_Project);

		m_UIManager.draw(m_MenuBar, m_Editor, m_Project);


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