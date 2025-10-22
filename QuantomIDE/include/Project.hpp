#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_set>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp> 

//JSON file structure
//{
//    "name": "MyProject",
//        "projectFilePath" : "path/to/projectfile.qproj",
//        "rootDirectory" : "path/to/project/root",
//
//        "sourceFiles" : [
//            "src/main.cpp",
//            "src/utils.cpp",
//            "src/module/submodule.cpp"
//        ] ,
//
//        "includeDirs": [
//            "include",
//            "external/libs",
//            "third_party"
//        ] ,
//
//        "compiler": "g++",
//        "compilerFlags" : [
//            "-std=c++20",
//            "-O2",
//            "-Wall"
//        ] ,
//
//        "openFiles": [
//            "src/main.cpp",
//            "src/utils.cpp"
//        ] ,
//
//        "dirty" : true
//}



class Project {
public:
	Project() = default;

	bool createNew(const std::filesystem::path& rootDir, const std::string& projectName);

	bool saveAs(const std::filesystem::path& newProjectFile);


	// Close project and clear data
	void close();
	bool isDirty() const { return dirty; }
	bool isOpen() const { return isopen; }
	bool open(const std::filesystem::path& path);
	bool save() ;

	void addResourceFile(const std::filesystem::path& file) {}
	void addHeaderFile(const std::filesystem::path& file){}
	void addSourceFile(const std::filesystem::path& file);
	void removeFile(const std::filesystem::path& file);
	bool containsFile(const std::filesystem::path& file) const;
	const std::filesystem::path getRootDirectory() const { return rootDirectory; }
	const std::unordered_map<std::filesystem::path, std::string>& getFileFilters() const { return fileToFilter; }
	const std::string& getName() const;
	const std::filesystem::path& getProjectPath() const;
	const std::vector<std::filesystem::path>& getSourceFiles() const;

private:
	std::string name;
	std::filesystem::path projectFilePath;
	std::filesystem::path rootDirectory;

	std::vector<std::filesystem::path> sourceFiles;
	std::vector<std::filesystem::path> includeDirs;
	std::unordered_map<std::filesystem::path, std::string> fileToFilter;



	std::string compiler;
	std::vector<std::string> compilerFlags;

	// UI state, dirty flags, etc.
	std::unordered_set<std::filesystem::path> openFiles;
	bool dirty = false;
	bool isopen = false;
};
