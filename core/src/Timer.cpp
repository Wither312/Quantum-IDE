#include "Timer.hpp"
#include "Platform.hpp"

ScopedTimer::ScopedTimer(const char* data) : m_Data(data), duration(0) {
    start = std::chrono::high_resolution_clock::now();
}

ScopedTimer::~ScopedTimer()
{
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;

    const float ms = duration.count() * 1000.0f;

    core::Platform::enableConsoleColors();
    core::Platform::printConsoleColor("INFO", m_Data, core::Color::Cyan, core::Color::Default, false);
    std::cout << ms << " ms" << std::endl;
}