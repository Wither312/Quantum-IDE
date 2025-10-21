#include <atomic>
#include <string>
#include <mutex>
#include <thread>
#include <filesystem>
#include <fstream>
#include <algorithm> // for std::transform
#include <stdexcept>
#include <vector>
#include <iostream>

#include "Project.hpp"
#include "EditorManager.hpp"


enum class CompileMode {
	CompileAndLink,  // default, no -c
	CompileOnly      // -c
};

enum class Compiler
{
	gcc,
	MSVC,
	Clang
};

enum class CompilerFlag {
	// C++ Standards
	Cpp98,
	Cpp03,
	Cpp11,
	Cpp14,
	Cpp17,
	Cpp20,
	Cpp23,

	// Optimization Levels
	Optimize0,  // -O0 (no optimization)
	Optimize1,  // -O1
	Optimize2,  // -O2
	Optimize3,  // -O3
	OptimizeS,  // -Os (optimize for size)
	OptimizeFast, // -Ofast (aggressive optimization, may break standards)

	// Warning Flags
	Wall,       // -Wall
	Wextra,     // -Wextra
	Werror,     // -Werror (treat warnings as errors)

	// Debugging
	Debug,      // -g (include debug symbols)

	// Sanitizers (runtime checks)
	AddressSanitizer, // -fsanitize=address
	ThreadSanitizer,  // -fsanitize=thread
	UndefinedBehaviorSanitizer, // -fsanitize=undefined

	// Misc
	Pedantic,   // -pedantic (strict standard compliance)
	StdLibcxx,  // -stdlib=libc++ (choose libc++ standard library, useful on some platforms)
	NoExceptions, // -fno-exceptions
	NoRtti       // -fno-rtti
};


static std::string to_string(CompilerFlag flag) {
	switch (flag) {
		// C++ standards
	case CompilerFlag::Cpp98: return "-std=c++98";
	case CompilerFlag::Cpp03: return "-std=c++03";
	case CompilerFlag::Cpp11: return "-std=c++11";
	case CompilerFlag::Cpp14: return "-std=c++14";
	case CompilerFlag::Cpp17: return "-std=c++17";
	case CompilerFlag::Cpp20: return "-std=c++20";
	case CompilerFlag::Cpp23: return "-std=c++23";

		// Optimization
	case CompilerFlag::Optimize0: return "-O0";
	case CompilerFlag::Optimize1: return "-O1";
	case CompilerFlag::Optimize2: return "-O2";
	case CompilerFlag::Optimize3: return "-O3";
	case CompilerFlag::OptimizeS: return "-Os";
	case CompilerFlag::OptimizeFast: return "-Ofast";

		// Warnings
	case CompilerFlag::Wall: return "-Wall";
	case CompilerFlag::Wextra: return "-Wextra";
	case CompilerFlag::Werror: return "-Werror";

		// Debug
	case CompilerFlag::Debug: return "-g";

		// Sanitizers
	case CompilerFlag::AddressSanitizer: return "-fsanitize=address";
	case CompilerFlag::ThreadSanitizer: return "-fsanitize=thread";
	case CompilerFlag::UndefinedBehaviorSanitizer: return "-fsanitize=undefined";

		// Misc
	case CompilerFlag::Pedantic: return "-pedantic";
	case CompilerFlag::StdLibcxx: return "-stdlib=libc++";
	case CompilerFlag::NoExceptions: return "-fno-exceptions";
	case CompilerFlag::NoRtti: return "-fno-rtti";
	}
	return "";
}
static std::string to_string(CompileMode mode) {
	switch (mode) {
	case CompileMode::CompileAndLink: return "";
	case CompileMode::CompileOnly: return "-c";
	}
	return "";
}

static std::string parseCompiler(Compiler compiler)
{
	switch (compiler)
	{
	case Compiler::gcc:   return "g++";
	case Compiler::MSVC:  return "cl";     // MSVC compiler executable is `cl`
	case Compiler::Clang: return "clang";
	}
	return ""; // or throw, if you prefer
}

class BuildSystem {
public:
	BuildSystem();

	void BuildCurrentProject( EditorManager&, const Project& );
	void RunCurrentProject(const Project& p_Project);

	std::string BuildFlags(const std::vector<CompilerFlag>&);
	std::string BuildFiles(std::vector<std::filesystem::path>);




	//    std::vector<std::string> otherOptions = {
	//    "-Iinclude",       // include directory
	//    "-DDEBUG=1",       // macro define
	//    "-Llibs",          // library directory
	//    "-lm"              // math lib
	//    };
	//
private:
	Compiler m_Compiler;
	std::vector<CompilerFlag> m_BuildFlags;



	char m_EditorBuffer[1024 * 16];
	std::atomic_bool m_IsBuilding; // ne se global, static znaci deka kje ima samo vo toj translation unit ili obj file, mislese deka se global zs pravese vo cpp, toa e samo init sto pravis vo cpp inc tuka se deklarirani
	std::string m_BuildOutput;
	std::mutex m_BuildMutex;

};