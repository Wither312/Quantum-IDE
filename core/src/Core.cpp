#include "Core.hpp"

//#include "EventBus.hpp"
//#include "FileSystem.hpp"
//#include "Events.hpp"
//#include "FileWatcher.hpp"
//#include "Logger.hpp"
//#include "MemoryPool.hpp"
//#include "Platform.hpp"
//#include "ThreadPool.hpp"
//#include "Timer.hpp"

using namespace core;

Core::Core()
	: m_logger(std::make_unique<Log>())
	, m_threadPool(std::make_unique<ThreadPool>())
	, m_memoryPool(std::make_unique<MemoryPool>())
	, m_eventBus(std::make_unique<EventBus>())
	, m_fileSystem(std::make_unique<FileSystem>())  // initialize here
{
	LOG("Core is initilazed", Log::LogLevel::Tracer);
}

Core::~Core() = default;

bool Core::initialize()
{
	// initialize subsystems...
	m_initialized = true;
	return m_initialized;
}

void Core::shutdown()
{
	// cleanup...
	m_initialized = false;
}

bool Core::isInitialized() const noexcept
{
	return m_initialized;
}

void Core::log(std::string_view message) const
{

}

core::Log* Core::getLogger() const noexcept { return m_logger.get(); }
ThreadPool* Core::getThreadPool() const noexcept { return m_threadPool.get(); }
MemoryPool* Core::getMemoryPool() const noexcept { return m_memoryPool.get(); }
EventBus* Core::getEventBus() const noexcept { return m_eventBus.get(); }
FileSystem* Core::getFileSystem() const noexcept { return m_fileSystem.get(); }
