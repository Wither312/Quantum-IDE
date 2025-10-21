#include "Project.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

// --- Project Loading ---

bool Project::createNew(const std::filesystem::path& rootDir, const std::string& pName)
{
    rootDirectory = rootDir;
    projectFilePath = std::filesystem::path(rootDir.string() + "\\" + pName);
    name = pName;
    isopen = true;

    return false;
}

bool Project::open(const fs::path& path) {
    if (!fs::exists(path)) return false;

    std::ifstream file(path);
    if (!file.is_open()) return false;

    json j;
    try {
        file >> j;
    }
    catch (const std::exception& e) {
        return false; // Failed to parse JSON
    }

    // Parse basic fields
    name = j.value("name", "");
    projectFilePath = path;
    rootDirectory = j.value("rootDirectory", path.parent_path().string());

    // Parse vectors of paths
    sourceFiles.clear();
    if (j.contains("sourceFiles") && j["sourceFiles"].is_array()) {
        for (const auto& s : j["sourceFiles"]) {
            sourceFiles.emplace_back(s.get<std::string>());
        }
    }

    includeDirs.clear();
    if (j.contains("includeDirs") && j["includeDirs"].is_array()) {
        for (const auto& d : j["includeDirs"]) {
            includeDirs.emplace_back(d.get<std::string>());
        }
    }

    compiler = j.value("compiler", "");
    compilerFlags.clear();
    if (j.contains("compilerFlags") && j["compilerFlags"].is_array()) {
        for (const auto& flag : j["compilerFlags"]) {
            compilerFlags.push_back(flag.get<std::string>());
        }
    }

    openFiles.clear();
    if (j.contains("openFiles") && j["openFiles"].is_array()) {
        for (const auto& f : j["openFiles"]) {
            openFiles.emplace(fs::path(f.get<std::string>()));
        }
    }

    dirty = j.value("dirty", false);
    
    isopen = true;
    return true;
}

bool Project::save() const { 
    std::ofstream file(std::string(this->projectFilePath.string() + ".qproj"));
    if (!file.is_open()) return false;

    json j;
    j["name"] = name;
    j["projectFilePath"] = projectFilePath.string();
    j["rootDirectory"] = rootDirectory.string();

    // Serialize vectors
    std::vector<std::string> srcFiles;
    for (const auto& src : sourceFiles)
        srcFiles.push_back(src.string());
    j["sourceFiles"] = srcFiles;

    std::vector<std::string> includes;
    for (const auto& inc : includeDirs)
        includes.push_back(inc.string());
    j["includeDirs"] = includes;

    j["compiler"] = compiler;
    j["compilerFlags"] = compilerFlags;

    std::vector<std::string> openFileStrings;
    for (const auto& f : openFiles)
        openFileStrings.push_back(f.string());
    j["openFiles"] = openFileStrings;

    j["dirty"] = dirty;

    file << j.dump(4); // pretty print with indent of 4 spaces

    return true;
}

// --- File Management ---

void Project::addSourceFile(const fs::path& file) {
    if (std::find(sourceFiles.begin(), sourceFiles.end(), file) == sourceFiles.end()) {
        sourceFiles.push_back(file);
        fileToFilter.insert({ file, "Source Files" });
        dirty = true;
    }
}

void Project::removeFile(const fs::path& file) {
    auto it = std::remove(sourceFiles.begin(), sourceFiles.end(), file);
    if (it != sourceFiles.end()) {
        sourceFiles.erase(it, sourceFiles.end());
        dirty = true;
    }
}

bool Project::containsFile(const fs::path& file) const {
    return std::find(sourceFiles.begin(), sourceFiles.end(), file) != sourceFiles.end();
}

// --- Getters ---

const std::vector<fs::path>& Project::getSourceFiles() const {
    return sourceFiles;
}

const std::string& Project::getName() const {
    return name;
}

const fs::path& Project::getProjectPath() const {
    return projectFilePath;
}
