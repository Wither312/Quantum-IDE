#include "Platform.hpp"

#include <iostream>

using namespace core;

#ifdef _WIN32
std::optional<std::filesystem::path> openFileDialogWin32(const char* filter = nullptr) {
	OPENFILENAMEA ofn;
	CHAR szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = filter ? filter : "All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn) == TRUE) {
		return std::filesystem::path(szFile);
	}
	return std::nullopt;
}
std::optional<std::filesystem::path> saveFileDialogWin32(const char* filter = nullptr) {
	OPENFILENAMEA ofn;
	CHAR szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = filter ? filter : "All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

	if (GetSaveFileNameA(&ofn) == TRUE) {
		return std::filesystem::path(szFile);
	}
	return std::nullopt;
}
std::optional<std::filesystem::path> folderDialogWin32() {
	wchar_t folderPath[MAX_PATH] = { 0 };

	BROWSEINFOW bi = { 0 };
	bi.lpszTitle = L"Select Folder";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
	if (pidl != nullptr) {
		if (SHGetPathFromIDListW(pidl, folderPath)) {
			CoTaskMemFree(pidl);
			// Convert wide string to std::filesystem::path
			return std::filesystem::path(folderPath);
		}
		CoTaskMemFree(pidl);
	}
	return std::nullopt;
}

#endif // _WIN32

#ifdef __linux__
std::optional<std::filesystem::path> openFileDialogLinux() {
	if (!gtk_init_check(0, nullptr))
		return std::nullopt;

	GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File",
		nullptr,
		GTK_FILE_CHOOSER_ACTION_OPEN,
		("_Cancel"), GTK_RESPONSE_CANCEL,
		("_Open"), GTK_RESPONSE_ACCEPT,
		nullptr);

	std::optional<std::filesystem::path> result;

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (file_path) {
			result = std::filesystem::path(file_path);
			g_free(file_path);
		}
	}

	gtk_widget_destroy(dialog);
	while (g_main_context_pending(nullptr))
		g_main_iteration(false);

	return result;
}

static std::optional<std::filesystem::path> saveFileDialogLinux() {
	if (!gtk_init_check(0, nullptr))
		return std::nullopt;

	GtkWidget* dialog = gtk_file_chooser_dialog_new("Save File",
		nullptr,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		("_Cancel"), GTK_RESPONSE_CANCEL,
		("_Save"), GTK_RESPONSE_ACCEPT,
		nullptr);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	std::optional<std::filesystem::path> result;

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (file_path) {
			result = std::filesystem::path(file_path);
			g_free(file_path);
		}
	}

	gtk_widget_destroy(dialog);
	while (g_main_context_pending(nullptr))
		g_main_iteration(false);

	return result;
}

static std::optional<std::filesystem::path> folderDialogLinux() {
	if (!gtk_init_check(0, nullptr))
		return std::nullopt;

	GtkWidget* dialog = gtk_file_chooser_dialog_new("Select Folder",
		nullptr,
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		("_Cancel"), GTK_RESPONSE_CANCEL,
		("_Select"), GTK_RESPONSE_ACCEPT,
		nullptr);

	std::optional<std::filesystem::path> result;

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* folder_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (folder_path) {
			result = std::filesystem::path(folder_path);
			g_free(folder_path);
		}
	}

	gtk_widget_destroy(dialog);
	while (g_main_context_pending(nullptr))
		g_main_iteration(false);

	return result;
}

#endif // __linux__

// Public API
std::optional<std::filesystem::path> Platform::openFileDialog(const char* filters) {
#ifdef _WIN32
	return openFileDialogWin32(filters);
#elif defined(__linux__)
	return openFileDialogLinux();
#else
	return std::nullopt;
#endif
}

std::optional<std::filesystem::path> Platform::saveFileDialog(const char* filters) {
#ifdef _WIN32
	return saveFileDialogWin32(filters);
#elif defined(__linux__)
	return saveFileDialogLinux();
#else
	return std::nullopt;
#endif
}

std::optional<std::filesystem::path> Platform::folderDialog() {
#ifdef _WIN32
	return folderDialogWin32();
#elif defined(__linux__)
	return folderDialogLinux();
#else
	return std::nullopt;
#endif
}

void Platform::enableConsoleColors() {
#ifdef WIN32
	HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hcon == INVALID_HANDLE_VALUE) return;

	DWORD dwMode = 0;
	if (!GetConsoleMode(hcon, &dwMode)) return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hcon, dwMode);
#endif
}

std::string Platform::getAnsiCode(const Color color) {
	switch (color) {
		case Color::Red:     return "\033[31m";
		case Color::Green:   return "\033[32m";
		case Color::Yellow:  return "\033[33m";
		case Color::Blue:    return "\033[34m";
		case Color::Magenta: return "\033[35m";
		case Color::Cyan:    return "\033[36m";
		case Color::White:   return "\033[37m";
		default:             return "\033[0m"; // reset
	}
}

void Platform::printConsoleColor(const std::string& text, const Color color, const bool newLine) {
	std::cout << getAnsiCode(color) << "[" << text << "]: " << getAnsiCode(Color::Default);
	if (newLine) std::cout << std::endl;
}

void Platform::printConsoleColor(const std::string& tag, const std::string& text, const Color tagColor, const Color messageColor, const bool newLine) {
	std::cout
		<< getAnsiCode(tagColor) << "[" << tag << "]: "
		<< getAnsiCode(messageColor) << text
		<< getAnsiCode(Color::Default);
	if (newLine) std::cout << std::endl;
}

void Platform::setConsoleColor(const Color color) {
	std::cout << getAnsiCode(color);
}

void Platform::resetConsoleColor() {
	std::cout << getAnsiCode(Color::Default);
}

