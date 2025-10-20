#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <optional>

namespace core {
    class Platform;
}


namespace core {

    class FileSystem {
    public:
        static bool exists(const std::filesystem::path& path);
        static bool createDirectory(const std::filesystem::path& path);
        static bool remove(const std::filesystem::path& path);

        static std::optional<std::string> readFile(const std::filesystem::path& filePath);
        static bool writeFile(const std::filesystem::path& filePath, std::string_view content);

        static std::vector<std::filesystem::path> listFiles(const std::filesystem::path& directory, bool recursive = false);
        std::optional<std::filesystem::path> openFile();
        std::optional<std::filesystem::path> saveFile();
        std::optional<std::filesystem::path> openFolder();
    };

} // namespace core
