#pragma once
#include <string>
#include <vector>
#include <map>
#include <Log.hpp>
#include <filesystem>



// --------------------
// Basic enums
// --------------------
enum class BreakpointType
{
	LINE,
	FUNCTION,
	CONDITIONAL,
	WATCH
};

enum class BreakpointStatus
{
	ENABLED,
	DISABLED,
	HIT
};

struct Variable
{
	std::string name;
	std::string type;
	std::string value;
};

struct Breakpoint
{
	std::string file;
	int line;
	BreakpointType type;
	BreakpointStatus status;
	std::string condition; // optional for conditional breakpoints
	std::vector<Variable> locals; // captured locals when breakpoint hits
};

// --------------------
// Core DebugSystem
// --------------------
class DebugSystem
{
public:
	DebugSystem();
	~DebugSystem();

	// ----------------
	// Program control
	// ----------------
	int loadExecutable(std::filesystem::path filePath);
	void run();
	void step();          // step over
	void next();          // step to next line
	void continueExecution();
	void stop();

	// ----------------
	// Breakpoints
	// ----------------
	void addBreakpoint(const std::string& file, int line);
	void removeBreakpoint(const std::string& file, int line);
	void enableBreakpoint(const std::string& file, int line);
	void disableBreakpoint(const std::string& file, int line);

	// ----------------
	// Variables
	// ----------------
	std::vector<Variable> getLocalVariables(); // at current breakpoint

	// ----------------
	// Helpers / state
	// ----------------
	bool isRunning() const { return m_running; }
	bool isPaused() const {return m_paused;}

private:
	std::filesystem::path m_executablePath;
	std::vector<Breakpoint> m_breakpoints;
	std::string m_DebugTask;

	bool m_running;
	bool m_paused;

	// internal helper
	Breakpoint* findBreakpoint(const std::string& file, int line);
};

