#pragma once

#include <iostream>
#include <chrono>
#include <string>
#include <ctime>
#include <cstdarg>
#include <iomanip>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _DEBUG
#define LOG(fmt, level, ...) Log::lOG(fmt, level, __VA_ARGS__)
#define TIMER(x) ScopedTimer timer(x)
#else
#define LOG(fmt, level, ...)
#define TIMER(x)
#endif
namespace core
{

	class Log
	{
	public:
		enum LogLevel {
			Error = 0,
			Warn = 1,
			Tracer = 2
		};

	private:
		inline static int m_LogLevel = Tracer;

	public:
		template<typename... Args>
		static void lOG(const char* fmt, LogLevel level, Args... args)
		{
			char buffer[1024];
			snprintf(buffer, sizeof(buffer), fmt, args...);
			LoG(buffer, level);
		}

		static void LoG(const char* formattedMsg, LogLevel severity);


		static void SetLogLevel(LogLevel level) {
			m_LogLevel = level;
		}
	};

}