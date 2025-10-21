#pragma once

#include <string>
#include <filesystem>

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

class FileDialog {
public:
	static std::string openFileDialog(const char* filter = nullptr) {
#ifdef _WIN32
		OPENFILENAMEA ofn;
		CHAR szFile[MAX_PATH] = { 0 };

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		if (filter)
			ofn.lpstrFilter = filter;
		else
			ofn.lpstrFilter = "All Files\0*.*\0";

		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE) {
			return std::string(szFile);
		}
		return std::string();

#elif defined(__linux__)
		std::string filename;

		if (!gtk_init_check(0, nullptr)) {
			return {};
		}

		GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File",
			nullptr,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			("_Cancel"), GTK_RESPONSE_CANCEL,
			("_Open"), GTK_RESPONSE_ACCEPT,
			nullptr);

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
			char* file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			if (file_path) {
				filename = file_path;
				g_free(file_path);
			}
		}

		gtk_widget_destroy(dialog);
		while (g_main_context_pending(nullptr))
			g_main_iteration(false);

		return filename;
#endif
	}

	static std::string saveFileDialog(const char* filter = nullptr) {
#ifdef _WIN32
		OPENFILENAMEA ofn;
		CHAR szFile[MAX_PATH] = { 0 };

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		if (filter)
			ofn.lpstrFilter = filter;
		else
			ofn.lpstrFilter = "All Files\0*.*\0";

		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE) {
			return std::string(szFile);
		}
		return std::string();

#elif defined(__linux__)
		std::string filename;

		if (!gtk_init_check(0, nullptr)) {
			return {};
		}

		GtkWidget* dialog = gtk_file_chooser_dialog_new("Save File",
			nullptr,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			("_Cancel"), GTK_RESPONSE_CANCEL,
			("_Save"), GTK_RESPONSE_ACCEPT,
			nullptr);

		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
			char* file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			if (file_path) {
				filename = file_path;
				g_free(file_path);
			}
		}

		gtk_widget_destroy(dialog);
		while (g_main_context_pending(nullptr))
			g_main_iteration(false);

		return filename;
#endif

	}
#ifdef _WIN32
	static std::filesystem::path openFolderDialog()
	{
		wchar_t folderPath[MAX_PATH] = { 0 };

		BROWSEINFOW bi = { 0 };                     // Use wide version
		bi.lpszTitle = L"Select Folder";           // Wide string is now correct

		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

		LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
		if (pidl != nullptr)
		{
			if (SHGetPathFromIDListW(pidl, folderPath))
			{
				// folderPath now holds the selected folder path
			}
			CoTaskMemFree(pidl);
		}

		return std::filesystem::path(folderPath);
	}
#endif
};

