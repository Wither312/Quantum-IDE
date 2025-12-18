// EditorManager.hpp
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <filesystem>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <sstream>
#include <optional>

#include "FileManager.hpp"
#include "LSP.hpp"
#include "imgui_internal.h"

// Forward declarations
class Document;
class SyntaxHighlighter;
class EditorTab;
class TabBar;

// Represents the text buffer and handles undo/redo
class Document {
public:
	Document() = default;
	~Document() = default;

	void setText(const std::string& text);
	std::string& getText();

	void undo();
	void redo();

	void insertText(size_t position, const std::string& text);
	void insertTextAtCursor(const std::string& text);

	bool isDirty() const { return m_Dirty; }
	void markClean() { m_Dirty = false; }

	void setCursorPos(size_t pos);
	std::pair<size_t, size_t> getCursorPos() const; // returns cursor line then column
	size_t getCursorIndex() const { return m_CursorPos; }
private:
	std::string m_TextBuffer;
	bool m_Dirty = false;

	std::vector<std::string> m_UndoStack;
	std::vector<std::string> m_RedoStack;

	size_t m_CursorPos = 0;
};

// Handles syntax highlighting of text
class SyntaxHighlighter { // TODO replace with the GitHub one
public:
	SyntaxHighlighter();
	~SyntaxHighlighter();

	// Apply syntax highlighting to a document
	void highlight(const Document& doc);

private:
	// Internal data/methods to handle syntax rules
};

// Represents a single tab in the editor, managing a document and highlighter
class EditorTab {
public:
	EditorTab(std::string );
	EditorTab(std::unique_ptr<Document> doc);
	~EditorTab();

	bool getFocusEditorNextFrame() const { return m_focusEditorNextFrame; }
	void setFocusEditorNextFrame(bool value) { m_focusEditorNextFrame = value; }
	void setTabName(std::string name) { m_TabName = name; }
	void setFilePath(std::filesystem::path path) { m_Path = path; }
	std::optional<std::filesystem::path> save();
	std::filesystem::path getFilePath()
	{
		if (this == nullptr) {
			std::cerr << "EditorTab 'this' pointer is null!\n";
			#ifdef MSVC
			__debugbreak();
			#endif
		}
		if (m_Path.empty() && !m_TabName.empty())
		{
			//TODO only call on tab not always
			return std::filesystem::path();
		}
		return m_Path;
	}

	void insertText(const std::string& text);

	void setID(std::string& id) { m_UniqueID = id; }
	std::string getID() { return m_UniqueID; }
	Document& getDocument();
	std::string& getTabName() { return m_TabName; }
	SyntaxHighlighter& getSyntaxHighlighter();

private:
	std::string m_UniqueID;
	std::string m_TabName;
	std::filesystem::path m_Path = "";
	SyntaxHighlighter m_SyntaxHighlighter;
	std::unique_ptr<Document> m_Document;
	bool m_focusEditorNextFrame = false;
};

// Manages the collection of tabs
class TabBar {
public:
	TabBar();
	~TabBar();

	void addTab(std::unique_ptr<EditorTab> tab);
	void closeTab(int index);
	void closeAll();
	void saveAll();
	EditorTab* getTab(int index);

	int getTabCount() const;

	// New:
	int getCurrentTabIndex() const;
	EditorTab* getCurrentTab();

	void setCurrentTabIndex(int index);

	void insertText(const std::string& text);

private:
	std::vector<std::unique_ptr<EditorTab>> m_Tabs;
	int m_CurrentTabIndex = -1;  // -1 means no active tab
};


// Main manager class for the editor subsystem
class EditorManager {
public:
	EditorManager();
	~EditorManager();

	void openFile(const std::string& filepath);

	void closeFile(int tabIndex);

	void insertText(const std::string& text);

	TabBar& getTabBar();

	void updateAutosave(float deltaTimeSeconds);
	void setAutosaveInterval(float seconds) { m_autoSaveInterval = seconds; }
	void setAutosaveEnabled(bool enabled) { m_autoSaveEnabled = enabled; }

private:
	TabBar m_TabBar;

	bool m_autoSaveEnabled		= false;
	float m_autoSaveInterval	= 60.0f;
	float m_autoSaveTimer		= 0.0f;
};
