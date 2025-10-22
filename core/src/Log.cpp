#include "Log.hpp"
using namespace core;

void Log::LoG(const char* formattedMsg, LogLevel severity)
{
    if (severity > m_LogLevel) return;

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

#ifdef _WIN32
    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD color = 7;
    switch (severity) {
    case Error:  color = 4; break;
    case Warn:   color = 14; break;
    case Tracer: color = 10; break;
    }
    SetConsoleTextAttribute(hcon, color);
#else
    const char* color = "";
    switch (severity) {
    case Error:  color = "\033[31m"; break;
    case Warn:   color = "\033[33m"; break;
    case Tracer: color = "\033[32m"; break;
    }
    std::cout << color;
#endif

    const char* levelStr = (severity == Error ? "ERROR" : severity == Warn ? "WARN" : "INFO");
    std::cout << "[" << levelStr << "][" << std::ctime(&time) << "]: " << formattedMsg << std::endl;

#ifdef _WIN32
    SetConsoleTextAttribute(hcon, 7);
#else
    std::cout << "\033[0m";
#endif
}
