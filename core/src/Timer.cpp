#include "Timer.hpp"
#include "Platform.hpp"

ScopedTimer::ScopedTimer(const char* data) : m_Data(data), duration(0) {
    start = std::chrono::steady_clock::now();
}

ScopedTimer::~ScopedTimer()
{
    end = std::chrono::steady_clock::now();
    duration = end - start;

    const long long ms = duration_cast<std::chrono::milliseconds>(end - start).count();

    core::Platform::enableConsoleColors();
    core::Platform::printConsoleColor("INFO", m_Data, core::Color::Cyan, core::Color::Default, false);
    std::cout << ms << " ms" << std::endl;
}