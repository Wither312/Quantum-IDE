#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "EventBus.hpp"
#include "FileSystem.hpp"
#include "Events.hpp"
#include "FileWatcher.hpp"
#include "Log.hpp"
#include "MemoryPool.hpp"
#include "Platform.hpp"
#include "ThreadPool.hpp"
#include "Timer.hpp"


namespace core {

    class Log;
    class ThreadPool;
    class MemoryPool;
    class EventBus;
    class FileSystem;

    class Core {
    public:
        Core();
        ~Core();

        Core(const Core&) = delete;
        Core& operator=(const Core&) = delete;

        bool initialize();
        void shutdown();
        [[nodiscard]] bool isInitialized() const noexcept;

        void log(std::string_view message) const;

        Log* getLogger() const noexcept;
        ThreadPool* getThreadPool() const noexcept;
        MemoryPool* getMemoryPool() const noexcept;
        EventBus* getEventBus() const noexcept;
        FileSystem* getFileSystem() const noexcept;

    private:
        bool m_initialized = false;

        std::unique_ptr<Log> m_logger;
        std::unique_ptr<ThreadPool> m_threadPool;
        std::unique_ptr<MemoryPool> m_memoryPool;
        std::unique_ptr<EventBus> m_eventBus;
        std::unique_ptr<FileSystem> m_fileSystem;    // << add FileSystem here
    };

} // namespace core
