#include "BuildSystem.hpp"


BuildSystem::BuildSystem()
{
	m_Compiler = Compiler::gcc;
	m_BuildFlags.push_back(CompilerFlag::Cpp20);
}

void BuildSystem::BuildCurrentProject(EditorManager& p_Editor, const Project& p_Project)
{
	if (m_IsBuilding.exchange(true)) {
		return;
	}

	// Copy editor buffer so thread doesn't capture ref to it
	std::string editorContent(m_EditorBuffer);

	if (p_Project.isDirty())
	{
		std::cerr << "[Warning]:Project not saved, saving" << std::endl;
		p_Editor.getTabBar().saveAll();
		p_Project.save();

	}

	std::thread([&]() {

		//{
		//	std::ofstream ofs(src);  // corrected mode
		//	std::lock_guard<std::mutex> lk(m_BuildMutex);
		//	ofs << editorContent;
		//} // mutex unlocked here   // ova go pravie file da bide prazen? zs voopsto citas?



		std::string otherOptions = "";
		std::string buildTask = "cd \"" + p_Project.getRootDirectory().string() + "\"" +
			" && " + parseCompiler(m_Compiler) + " " +
			BuildFlags(m_BuildFlags) + " " +
			BuildFiles(p_Project.getSourceFiles()) + " " +
			otherOptions + " -o \"" + p_Project.getRootDirectory().string() + "\\" + p_Project.getName() + ".exe\"";


		std::string output;
#ifdef _WIN32
		FILE* pipe = _popen(buildTask.c_str(), "r");
#else
		FILE* pipe = popen(buildTask.c_str(), "r");
#endif
		if (pipe) {
			char buf[512];
			while (fgets(buf, sizeof(buf), pipe)) {
				output += buf;
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
			std::lock_guard<std::mutex> lk(m_BuildMutex);
			BuildSystem::m_BuildOutput = output;
			std::fstream stdOut("output.txt", std::ios::out);
			stdOut << output;
			stdOut.close();
		}

		m_IsBuilding.store(false);
		}).detach();
}
void BuildSystem::RunCurrentProject(const Project& p_Project)
{
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

	std::cout << result << std::endl;
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