// EditorManager.cpp

#include "EditorManager.hpp"

#include <Core.hpp>

// -------- Document --------

void Document::setText(const std::string& text)
{
	m_TextBuffer = text;
	m_UndoStack.clear();
	m_RedoStack.clear();
}

std::string& Document::getText()
{
	return m_TextBuffer;
}

void Document::undo()
{
	if (!m_UndoStack.empty())
	{
		m_RedoStack.push_back(m_TextBuffer);
		m_TextBuffer = m_UndoStack.back();
		m_UndoStack.pop_back();
	}
}

void Document::redo()
{
	if (!m_RedoStack.empty())
	{
		m_UndoStack.push_back(m_TextBuffer);
		m_TextBuffer = m_RedoStack.back();
		m_RedoStack.pop_back();
	}
}

// -------- SyntaxHighlighter --------
SyntaxHighlighter::SyntaxHighlighter() = default;
SyntaxHighlighter::~SyntaxHighlighter() = default;

void SyntaxHighlighter::highlight(const Document& /*doc*/)
{
	// TODO: Implement syntax highlighting logic
}

// -------- EditorTab --------
EditorTab::EditorTab(std::string name)
	: m_TabName(name), m_Document(std::make_unique<Document>()), m_Path("")
{
}

EditorTab::EditorTab(std::unique_ptr<Document> doc)
	: m_Document(std::move(doc)), m_Path("")
{
}

EditorTab::~EditorTab() = default;

Document& EditorTab::getDocument()
{
	return *m_Document;
}

SyntaxHighlighter& EditorTab::getSyntaxHighlighter()
{
	return m_SyntaxHighlighter;
}

// -------- TabBar --------
TabBar::TabBar() = default;
TabBar::~TabBar() = default;

void TabBar::addTab(std::unique_ptr<EditorTab> tab)
{
	m_Tabs.push_back(std::move(tab));
}

void TabBar::closeTab(int index)
{
	if (index >= 0 && index < static_cast<int>(m_Tabs.size()))
	{
		m_Tabs.erase(m_Tabs.begin() + index);
		m_Tabs.shrink_to_fit();
	}
}

EditorTab* TabBar::getTab(int index)
{
	if (index >= 0 && index < static_cast<int>(m_Tabs.size()))
		return m_Tabs[index].get();
	return nullptr;
}

int TabBar::getTabCount() const
{
	return static_cast<int>(m_Tabs.size());
}

int TabBar::getCurrentTabIndex() const {
	return m_CurrentTabIndex;
}

EditorTab* TabBar::getCurrentTab() {
	if (m_CurrentTabIndex >= 0 && m_CurrentTabIndex < static_cast<int>(m_Tabs.size()))
		return m_Tabs[m_CurrentTabIndex].get();
	return nullptr;
}

void TabBar::setCurrentTabIndex(int index) {
	if (index >= 0 && index < static_cast<int>(m_Tabs.size()))
		m_CurrentTabIndex = index;
	else
		m_CurrentTabIndex = -1;
}


// -------- EditorManager --------
EditorManager::EditorManager() = default;
EditorManager::~EditorManager() = default;

void EditorManager::openFile(const std::string& filepath)
{
	std::ifstream file(filepath);
	if (!file)
	{
		std::cerr << "Failed to open file: " << filepath << std::endl;
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	auto doc = std::make_unique<Document>();
	doc->setText(buffer.str());

	auto tab = std::make_unique<EditorTab>(std::move(doc));
	tab.get()->setTabName(std::filesystem::path(filepath).filename().string());
	tab.get()->setFilePath(std::filesystem::path(filepath));
	m_TabBar.addTab(std::move(tab));

	m_TabBar.setCurrentTabIndex(m_TabBar.getTabCount() - 1);
}

void EditorManager::closeFile(int tabIndex)
{
	m_TabBar.closeTab(tabIndex);
}

TabBar& EditorManager::getTabBar()
{
	return m_TabBar;
}
void TabBar::saveAll()
{
	for (auto& tab : m_Tabs)
	{

		if (!tab) {
			std::cerr << "[Warning] Tab does not exist.\n";
		}
		else {
			if (tab->getFilePath().empty())
			{
				extern core::Core g_Core;
				if (!g_Core.getFileSystem()->saveFile(tab->getFilePath()).has_value())
				{
					std::cerr << "Couldn't save file\n";
				}
				std::filesystem::path filePath = g_Core.getFileSystem()->saveFile(tab->getFilePath()).value();
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
