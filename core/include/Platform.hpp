#pragma once

#include <optional>
#include <filesystem>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <ShlObj.h>
#elif defined(__linux__)
#include <gtk/gtk.h>
#else
#error Platform not supported
#endif

namespace core
{
	enum class Color {
		Default,
		Red,
		Green,
		Blue,
		Yellow,
		Magenta,
		Cyan,
		White
	};

	class Platform {
	public:
		Platform() = delete;
		Platform(const Platform&) = delete;
		Platform& operator=(const Platform&) = delete;
		static std::optional<std::filesystem::path> openFileDialog(const char* filters = nullptr);
		static std::optional<std::filesystem::path> saveFileDialog(const char* filters = nullptr);
		static std::optional<std::filesystem::path> folderDialog();

		// Console color functions
		static void enableConsoleColors();
		static std::string getAnsiCode(Color color);
		static void printConsoleColor(const std::string& text, Color color = Color::Default, bool newLine = true);
		static void printConsoleColor(const std::string& tag, const std::string& text, Color tagColor = Color::Cyan, Color messageColor = Color::Default, bool newLine = true);
		static void setConsoleColor(Color color);
		static void resetConsoleColor();
	};

}