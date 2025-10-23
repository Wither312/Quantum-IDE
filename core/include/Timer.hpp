#pragma once

#include <iostream>
#include <chrono>

class ScopedTimer
{
private:
	const char* m_Data;
public:
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;
	explicit ScopedTimer(const char* data);
	~ScopedTimer();
};

