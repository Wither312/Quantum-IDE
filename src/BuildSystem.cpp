#include "BuildSystem.hpp"

char BuildSystem::g_EditorBuffer[1024 * 16] = {};
std::atomic_bool BuildSystem::g_IsBuilding = false;
std::string BuildSystem::g_BuildOutput;
std::mutex BuildSystem::g_BuildMutex;

void BuildSystem::BuildCurrentFile() {
    if (g_IsBuilding.exchange(true)) {
        return;
    }

    std::thread([]() {
        namespace fs = std::filesystem;
        fs::path src = fs::current_path() / "temp_build.cpp";
        {
            std::ofstream ofs(src, std::ios::binary);
            std::lock_guard<std::mutex> lk(BuildSystem::g_BuildMutex);
            ofs << g_EditorBuffer; // write current editor contents
        }

        // Use g++ (or change to gcc if you prefer). Redirect stderr to stdout.
        std::string cmd = "g++ -std=c++17 -O2 -o build.exe \"" + src.string() + "\" 2>&1";

        std::string output;
#ifdef _WIN32
        FILE* pipe = _popen(cmd.c_str(), "r");
#else
        FILE* pipe = popen(cmd.c_str(), "r");
#endif
        if (pipe) {
            char buf[512];
            while (fgets(buf, sizeof(buf), pipe)) {
                output += buf;
            }
#ifdef _WIN32
            _pclose(pipe);
#else
            pclose(pipe);
#endif
        } else {
            output = "Failed to start compiler process\n";
        }

        {
            std::lock_guard<std::mutex> lk(g_BuildMutex);
            BuildSystem::g_BuildOutput = output;
        }
        g_IsBuilding.store(false);
    }).detach();
}