#include "BuildSystem.hpp"


BuildSystem::BuildSystem()
{
	m_Compiler = Compiler::gcc;
	m_BuildFlags.push_back(CompilerFlag::Cpp20);
}

void BuildSystem::BuildCurrentFile(const std::string& fileName) {
	if (m_IsBuilding.exchange(true)) {
		return;
	}

	// Copy editor buffer so thread doesn't capture ref to it
	std::string editorContent(m_EditorBuffer);

	if (fileName.empty())
	{
		std::cerr << "[Warning]:Filename is empty" << std::endl;
	}

	std::thread([&]() {

		//{
		//	std::ofstream ofs(src);  // corrected mode
		//	std::lock_guard<std::mutex> lk(m_BuildMutex);
		//	ofs << editorContent;
		//} // mutex unlocked here   // ova go pravie file da bide prazen? zs voopsto citas?
		m_Sources.clear();

		m_Sources.push_back(fileName); // Consider protecting m_Sources with mutex

		std::string otherOptions = "";
		std::string buildTask = parseCompiler(m_Compiler) + " " + BuildFlags(m_BuildFlags) + " " + BuildFiles() + " " + otherOptions + " -o " + fileName + ".exe";

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
std::string BuildSystem::BuildFlags(const std::vector<CompilerFlag>& flags)
{
	std::string compilerFlags;
	for (auto f : flags) {
		compilerFlags += " " + to_string(f);
	}
	return compilerFlags;
}
std::string BuildSystem::BuildFiles()
{
	std::string inputFiles;
	for (const auto& src : m_Sources) {
		inputFiles += " " + src;
	}
	return inputFiles;
}