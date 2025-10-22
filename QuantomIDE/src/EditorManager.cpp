// EditorManager.cpp
#include "EditorManager.hpp"
#include <Core.hpp>
extern core::Core g_Core;



static std::string GenerateRandomID()
{
	static std::mt19937 rng(std::random_device{}());
	static std::uniform_int_distribution<int> dist(0, 15);

	std::stringstream ss;
	ss << std::hex;
	for (int i = 0; i < 8; ++i) {
		ss << dist(rng);
	}
	return ss.str(); // e.g., "a3f9b12e"
}

// -------- Document --------

void Document::setText(const std::string& text)
{
	m_TextBuffer = text;
	m_UndoStack.clear();
	m_RedoStack.clear();
	m_Dirty = true;
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
	std::string id;

	do {
		id = GenerateRandomID();
	} while (std::any_of(m_Tabs.begin(), m_Tabs.end(),
		[&](const std::unique_ptr<EditorTab>& existingTab) {
			return existingTab->getID() == id;
		}));

	tab->setID(id);
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
			LOG("Tab does not exist", core::Log::LogLevel::Warn);
		}
		else {
			auto path = tab->save();
			if (!path.has_value())
			{
				LOG("Warning: File failed to save.", core::Log::LogLevel::Warn);
				continue;
			}
			tab->setFilePath(path.value());
			tab->setTabName(std::filesystem::path(path.value()).filename().string());
			tab->getDocument().markClean();
		}
	}
}
std::optional<std::filesystem::path> EditorTab::save()
{
	auto buffer = m_Document.get()->getText();
	auto path = g_Core.getFileSystem()->saveFile(buffer, m_Path);
	if (path.has_value())
	{

		setTabName(path.value().filename().string());
		setFilePath(path.value());
		m_Document.get()->markClean();
		return std::optional<std::filesystem::path>(m_Path);
	}
	LOG("Warning: File save operation failed or was cancelled by user.", core::Log::LogLevel::Warn);
	return std::nullopt;
}
void TabBar::closeAll()
{
	m_Tabs.clear();
	m_Tabs.shrink_to_fit();
}