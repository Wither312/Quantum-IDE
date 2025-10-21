#include "Platform.hpp"
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

std::optional<std::filesystem::path> Platform::openFileDialog() {
#ifdef _WIN32
	return openFileDialogWin32();
#elif defined(__linux__)
	return openFileDialogLinux();
#else
	return std::nullopt;
#endif
}

std::optional<std::filesystem::path> Platform::saveFileDialog() {
#ifdef _WIN32
	return saveFileDialogWin32();
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
