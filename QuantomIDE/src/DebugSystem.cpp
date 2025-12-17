#include "DebugSystem.hpp"

DebugSystem::DebugSystem() :
	m_running(false), m_paused(false)
{
	LOG("Debugging rebuild", core::Log::LogLevel::Tracer);
}
DebugSystem::~DebugSystem()
{
	LOG("Debugging has been stopped", core::Log::LogLevel::Tracer);
}
int DebugSystem::loadExecutable(std::filesystem::path filePath)
{
	if (filePath.empty())
	{
		LOG("[DebugSystem]: Executable filepath is empty", core::Log::LogLevel::Warn);
		return 1;
	}

#ifdef _WIN32
	filePath.concat(".exe");
#endif
	if (!std::filesystem::exists(filePath))
	{
		LOG("[DebugSystem]: Executable filepath does not exist", core::Log::LogLevel::Warn);
		return 1;
	}
	m_executablePath = filePath;
	LOG(std::string("[DebugSystem]: Built GDB command: " + m_DebugTask).c_str(), core::Log::LogLevel::Tracer);
	return 0;
}
void DebugSystem::run()
{

	std::ostringstream cmd;
	cmd << "gdb";

	// Add each breakpoint as a GDB command
	for (const auto& bp : m_breakpoints)
	{
		if (bp.status == BreakpointStatus::ENABLED)
			cmd << " --ex \"break " << bp.file << ":" << bp.line << "\"";
	}

	// Start the program automatically once loaded
	cmd << " --ex run";

	// Add the target executable
	cmd << " --args " << m_executablePath;

	std::string gdbCommand = cmd.str();

	LOG(std::string("[DebugSystem]: Running GDB command: " + gdbCommand).c_str(), core::Log::LogLevel::Tracer);

	int result = std::system(gdbCommand.c_str());
	if (result != 0)
	{
		LOG(std::string("[DebugSystem]: GDB exited with code " + std::to_string(result)).c_str(), core::Log::LogLevel::Warn);
	}
	else
	{
		m_running = true;
		m_paused = false;
		LOG(std::string("[DebugSystem]: Debug session started.").c_str(), core::Log::LogLevel::Tracer);
	}
}

void DebugSystem::step()
{
}
void DebugSystem::next()
{
}
inline void DebugSystem::continueExecution()
{
}
void DebugSystem::stop()
{
}
void DebugSystem::addBreakpoint(const std::string& file, int line)
{
	Breakpoint bp;
	bp.type = BreakpointType::LINE;
	bp.file = file;
	bp.line = line;
	bp.status = BreakpointStatus::ENABLED;
	m_breakpoints.push_back(bp);
}
void DebugSystem::removeBreakpoint(const std::string& file, int line)
{
	auto it = std::find_if(
		m_breakpoints.begin(),
		m_breakpoints.end(),
		[&](const Breakpoint& bp)
		{
			return (bp.file == file && bp.line == line);
		}
	);

	if (it != m_breakpoints.end())
	{
		m_breakpoints.erase(it);
		LOG(std::string("[DebugSystem]: Removed breakpoint at " + file + ":" + std::to_string(line)).c_str(), core::Log::LogLevel::Tracer);
	}
	else
	{
		LOG(std::string("[DebugSystem]: No breakpoint found at " + file + ":" + std::to_string(line)).c_str(), core::Log::LogLevel::Warn);
	}
}

void DebugSystem::enableBreakpoint(const std::string& file, int line)
{
}

void DebugSystem::disableBreakpoint(const std::string& file, int line)
{
}
std::vector<Variable> DebugSystem::getLocalVariables()
{
	return std::vector<Variable>();
}
Breakpoint* DebugSystem::findBreakpoint(const std::string& file, int line)
{
	return nullptr;
}