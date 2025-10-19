#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <system_error>

class FileManager {
public:
    FileManager() = delete;
    ~FileManager() = delete;

    // File reading/writing
    static bool loadFileToString(const std::filesystem::path& filepath, std::string& outContent);
    static bool saveStringToFile(const std::filesystem::path& filepath, const std::string& content);

    // File existence and info

    static bool fileExists(const std::filesystem::path& filepath);
    static bool isDirectory(const std::filesystem::path& path) ;
    static uintmax_t fileSize(const std::filesystem::path& filepath);

    // Directory operations
    static bool createDirectory(const std::filesystem::path& dirPath, bool recursive = true);
    static bool removeFile(const std::filesystem::path& filepath);
    static bool removeDirectory(const std::filesystem::path& dirPath, bool recursive = false);

    // Optional: rename or move file
    static bool renameFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);

    // Optional: copy file
    static bool copyFile(const std::filesystem::path& source, const std::filesystem::path& destination, bool overwrite = false);

    static bool saveFile(const std::filesystem::path& filepath, const std::string& data);

private:
    // Helper to convert exceptions to bool failure for noexcept API
    template<typename Func>
    static bool safeExecute(Func&& func);
};
