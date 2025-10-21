#include "FileManager.hpp"


bool FileManager::loadFileToString(const std::filesystem::path& filepath, std::string& outContent)
{
	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	if (!file) return false;

	auto size = file.tellg();
	if (size < 0) return false;

	outContent.resize(static_cast<size_t>(size));
	file.seekg(0);
	file.read(outContent.data(), size);

	return file.good();
}

bool FileManager::saveStringToFile(const std::filesystem::path& filepath, const std::string& content)
{
	std::ofstream file(filepath, std::ios::binary);
	if (!file) return false;

	file.write(content.data(), content.size());
	return file.good();
}

bool FileManager::fileExists(const std::filesystem::path& filepath)
{
	return std::filesystem::exists(filepath);
}

bool FileManager::isDirectory(const std::filesystem::path& path)
{
	return std::filesystem::is_directory(path);
}

uintmax_t FileManager::fileSize(const std::filesystem::path& filepath)
{
	std::error_code ec;
	auto size = std::filesystem::file_size(filepath, ec);
	if (ec) return 0;
	return size;
}

bool FileManager::createDirectory(const std::filesystem::path& dirPath, bool recursive)
{
	std::error_code ec;
	bool created = false;

	if (recursive)
		created = std::filesystem::create_directories(dirPath, ec);
	else
		created = std::filesystem::create_directory(dirPath, ec);

	return !ec && (created || std::filesystem::exists(dirPath));
}

bool FileManager::removeFile(const std::filesystem::path& filepath) {
	std::error_code ec;
	bool removed = std::filesystem::remove(filepath, ec);
	return !ec && removed;
}

bool FileManager::removeDirectory(const std::filesystem::path& dirPath, bool recursive)
{
	std::error_code ec;
	bool removed = false;

	if (recursive)
		removed = std::filesystem::remove_all(dirPath, ec) > 0;
	else
		removed = std::filesystem::remove(dirPath, ec);

	return !ec && removed;
}

bool FileManager::renameFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath)
{
	std::error_code ec;
	std::filesystem::rename(oldPath, newPath, ec);
	return !ec;
}

bool FileManager::copyFile(const std::filesystem::path& source, const std::filesystem::path& destination, bool overwrite)
{
	std::error_code ec;
	auto options = overwrite ? std::filesystem::copy_options::overwrite_existing
		: std::filesystem::copy_options::none;

	std::filesystem::copy_file(source, destination, options, ec);
	return !ec;
}

bool FileManager::saveFile(const std::filesystem::path& filepath, const std::string& data) 
{
	std::ofstream file(filepath);
	if (!file.is_open()) {
		return false;  // Could not open file
	}
	file << data;
	return true;
}