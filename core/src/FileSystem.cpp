#include "FileSystem.hpp"
#include "Platform.hpp"

#include <fstream>
#include <vector>
#include <system_error>

using namespace core;

bool FileSystem::exists(const std::filesystem::path& path)
{
	return std::filesystem::exists(path);
}

bool FileSystem::createDirectory(const std::filesystem::path& path)
{
	std::error_code ec;
	bool created = std::filesystem::create_directory(path);
	return created && !ec;
}

bool FileSystem::remove(const std::filesystem::path& path)
{
	std::error_code ec;
	bool removed = std::filesystem::remove_all(path, ec) > 0;
	return removed && !ec;
}

std::optional<std::string> FileSystem::readFile(const std::filesystem::path& filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file)
		return std::nullopt;

	std::string contents;
	file.seekg(0, std::ios::end);
	contents.resize(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(&contents[0], contents.size());
	return contents;
}

bool FileSystem::writeFile(const std::filesystem::path& filePath, std::string_view content)
{
	std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
	if (!file)
		return false;

	file.write(content.data(), content.size());
	return file.good();
}

std::vector<std::filesystem::path> FileSystem::listFiles(const std::filesystem::path& directory, bool recursive)
{
	std::vector<std::filesystem::path> files;
	std::error_code ec;

	if (recursive)
	{
		for (auto& entry : std::filesystem::recursive_directory_iterator(directory, ec))
		{
			if (!ec && entry.is_regular_file())
				files.push_back(entry.path());
		}
	}
	else
	{
		for (auto& entry : std::filesystem::directory_iterator(directory, ec))
		{
			if (!ec && entry.is_regular_file())
				files.push_back(entry.path());
		}
	}
	return files;
}

std::optional<std::filesystem::path> FileSystem::openFile(std::filesystem::path path)
{
	return Platform::openFileDialog();
}

std::optional<std::filesystem::path> FileSystem::saveFile(std::string& buffer, std::filesystem::path path)
{
	if (!path.empty())
	{
		std::ofstream out(path);
		if (!out.is_open())
			return std::nullopt;

		out << buffer;
		return path;
	}
	else
	{
		auto maybePath = Platform::saveFileDialog();
		if (!maybePath)
			return std::nullopt;

		std::ofstream out(*maybePath);
		if (!out.is_open())
			return std::nullopt;

		out << buffer;
		return maybePath;
	}
}



std::optional<std::filesystem::path> FileSystem::openFolder(std::filesystem::path path)
{

	return Platform::folderDialog();
}
