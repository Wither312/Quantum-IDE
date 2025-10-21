#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include <chrono>
#include <unordered_map>

namespace core {

    class FileWatcher {
    public:
        using Callback = std::function<void(const std::filesystem::path&)>;

        FileWatcher() = default;
        ~FileWatcher();

        // Add path to watch, call callback on change
        void watch(const std::filesystem::path& path, Callback callback);

        // Remove path from watching
        void unwatch(const std::filesystem::path& path);

        // Poll for changes, should be called periodically (e.g. each frame or on a timer)
        void pollChanges();

    private:
        std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> m_paths;
        std::unordered_map<std::filesystem::path, Callback> m_callbacks;
    };

} // namespace core
