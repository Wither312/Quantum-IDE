#include "BuildSystem.hpp"

std::string BuildSystem::s_BuildOutput;
std::string BuildSystem::s_ConsoleOutput;

BuildSystem::BuildSystem()
{
	m_Compiler = Compiler::gcc;
	m_BuildFlags.push_back(CompilerFlag::Cpp20);
	m_BuildFlags.push_back(CompilerFlag::Optimize0);
	m_BuildFlags.push_back(CompilerFlag::Debug);
}

void BuildSystem::BuildCurrentProject(EditorManager& p_Editor, Project& p_Project)
{
	if (m_IsBuilding.exchange(true)) {
		// Already building, ignore subsequent calls
		return;
	}

	// Save project if dirty
	if (p_Project.isDirty())
	{
		LOG("[Warning]: Project not saved, saving...", core::Log::LogLevel::Warn);
		p_Editor.getTabBar().saveAll();
		p_Project.save();
	}

	std::thread([this, &p_Project]() {
		std::string output;

		std::string buildCommand =
			"cd \"" + p_Project.getRootDirectory().string() + "\""
			+ " && " + parseCompiler(m_Compiler) + " "
			+ BuildFlags(m_BuildFlags) + " "
			+ BuildFiles(p_Project.getSourceFiles()) + " "
			+ " -o \"" + p_Project.getRootDirectory().string() + "\\" + p_Project.getName() + ".exe\""
			+ " 2>&1"; // Redirect stderr to stdout

#ifdef _WIN32
		FILE* pipe = _popen(buildCommand.c_str(), "r");
#else
		FILE* pipe = popen(buildCommand.c_str(), "r");
#endif
		if (pipe) {
			char buffer[512];
			while (fgets(buffer, sizeof(buffer), pipe)) {
				output += buffer;
			}
#ifdef _WIN32
			_pclose(pipe);
#else
			pclose(pipe);
#endif
		}
		else {
			output = "Failed to start compiler process\n";
		}

		{
			std::lock_guard<std::mutex> lock(m_BuildMutex);
			s_BuildOutput = output;
			if (output.empty())
			{
				s_BuildOutput = "Build is successfull";
			}


		}

		m_IsBuilding.store(false);
		}).detach();
}

void BuildSystem::RunCurrentProject(const Project& p_Project)
{
	if (p_Project.getName().empty())
	{
		LOG("No project is open", core::Log::LogLevel::Warn);
		return;
	}
	std::string command = "\"" + p_Project.getRootDirectory().string() + "\\" + p_Project.getName() + "\"";  // Quote it in case of spaces

	std::string result;
	char buffer[128];

	// "r" = read the output of the command
#ifdef __linux__
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
#elif _WIN32
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
#endif
	if (!pipe.get())
		throw std::runtime_error("Failed to run command");

	while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
	{
		result += buffer;
	}
	s_ConsoleOutput = result;

	LOG("Console pipe output: %s", core::Log::LogLevel::Tracer, result.c_str());
}
std::string BuildSystem::BuildFlags(const std::vector<CompilerFlag>& flags)
{
	std::string compilerFlags;
	for (auto f : flags) {
		compilerFlags += " " + to_string(f);
	}
	return compilerFlags;
}
std::string BuildSystem::BuildFiles(std::vector<std::filesystem::path> p_Sources)
{
	std::string inputFiles;
	for (const auto& src : p_Sources) {
		inputFiles += " \"" + src.string() + "\"";
	}
	return inputFiles;
}