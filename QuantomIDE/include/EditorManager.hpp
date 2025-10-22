// EditorManager.hpp
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <filesystem>
#include <iostream>
#include "FileManager.hpp"

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

private:
	std::string m_TextBuffer;

	std::vector<std::string> m_UndoStack;
	std::vector<std::string> m_RedoStack;
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
	EditorTab(std::string);
	EditorTab(std::unique_ptr<Document> doc);
	~EditorTab();

	void setTabName(std::string name) { m_TabName = name; }
	void setFilePath(std::filesystem::path path) { m_Path = path; }

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
			std::cerr << "EditorTab file path is empty!\n";
			return std::filesystem::path();
		}
		return m_Path;
	}

	Document& getDocument();
	std::string& getTabName() { return m_TabName; }
	SyntaxHighlighter& getSyntaxHighlighter();

private:
	std::string m_TabName;
	std::filesystem::path m_Path = "";
	SyntaxHighlighter m_SyntaxHighlighter;
	std::unique_ptr<Document> m_Document;

};

// Manages the collection of tabs
class TabBar {
public:
	TabBar();
	~TabBar();

	void addTab(std::unique_ptr<EditorTab> tab);
	void closeTab(int index);
	void saveAll()
	{
		for (auto& tab : m_Tabs)
		{

			if (!tab) {
				std::cerr << "[Warning] Tab does not exist.\n";
			}
			else {
				if (tab->getFilePath().empty()) {
					
					std::string filePath = FileDialog::saveFileDialog();
					if (!filePath.empty()) { // user selected a path
						tab->setFilePath(filePath);
						tab->setTabName(std::filesystem::path(filePath).filename().string());
						FileManager::saveFile(filePath, tab->getDocument().getText());
					}
				}
				else {
					FileManager::saveFile(tab->getFilePath(), tab->getDocument().getText());
				}
			}
		}
	}
	EditorTab* getTab(int index);

	int getTabCount() const;

	// New:
	int getCurrentTabIndex() const;
	EditorTab* getCurrentTab();

	void setCurrentTabIndex(int index);

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

	TabBar& getTabBar();

private:
	TabBar m_TabBar;

};
