#pragma once
#include <iostream>
class ScopedTimer
{
private:
	const char* m_Data;
public:
	void WriteInColor(unsigned short color, std::string outputString)
	{
		HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hcon, color);
		std::cout << "[" << outputString << "]: ";
		SetConsoleTextAttribute(hcon, 7);
	}
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;
	ScopedTimer(const char* data) : m_Data(data), duration(0)
	{
		start = std::chrono::high_resolution_clock::now();
	}
	~ScopedTimer()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		float ms = duration.count() * 1000.0f;

		WriteInColor(10, m_Data);
		std::cout << ms << " ms" << std::endl;
	}
};

