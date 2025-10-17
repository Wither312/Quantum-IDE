#include <atomic>
#include <string>
#include <mutex>
#include <thread>
#include <filesystem>
#include <fstream>

class BuildSystem {
    static char g_EditorBuffer[1024 * 16];
    static std::atomic_bool g_IsBuilding;
    static std::string g_BuildOutput;
    static std::mutex g_BuildMutex;
    BuildSystem();
public:
    static void BuildCurrentFile();
};