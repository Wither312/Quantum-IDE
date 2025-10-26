#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <functional>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct CompletionItem {
    std::string label;
    std::string detail;
    std::string insertText;
};

class LSPClient {
public:
    using OnDiagnostics = std::function<void(const std::string& uri, const json& diagnostics)>;
    using OnCompletion = std::function<void(int id, const std::vector<CompletionItem>& items)>;
    using OnLog = std::function<void(const std::string& message)>;

    LSPClient(std::string serverPath = "clangd", std::vector<std::string> args = {});
    ~LSPClient();

    // Lifecycle
    bool start();
    void stop();

    // Requests
    int textDocumentCompletion(const std::string& uri, int line, int character);
    void textDocumentDidOpen(const std::string& uri, const std::string& languageId, const std::string& text);
    void textDocumentDidChange(const std::string& uri, const std::string& text);

    // Configuration
    void setOnDiagnostics(OnDiagnostics cb) { diagnosticsCB = std::move(cb); }
    void setOnCompletion(OnCompletion cb) { completionCB = std::move(cb); }
    void setOnLog(OnLog cb) { logCB = std::move(cb); }
private:
    std::string serverPath;
    std::vector<std::string> args;

#ifdef _WIN32
    // Windows handles
    void* processHandle = nullptr;
    void* threadHandle = nullptr;
    void* childStd_IN_Wr = nullptr;
    void* childStd_OUT_Rd = nullptr;
#else
    // POSIX
    int toChild_fd = -1;
    int fromChild_fd = -1;
    pid_t childPid = -1;
#endif

    std::thread readerThread;
    std::atomic<bool> running = false;

    std::mutex writeMutex;
    int idCounter = 1;

    std::unordered_map<int, int> pendingRequests;

    // Callbacks
    OnDiagnostics diagnosticsCB;
    OnCompletion completionCB;
    OnLog logCB;

    // helpers
#ifdef WIN32
    bool spawnClangdWin();
    bool writeRawWin(const std::string& s);
    void closeHandlesWin();
#else
    bool spawnClangdLinux();
    bool writeRawLinux(const std::string& s);
#endif

    void readerLoop();
    int nextId();
    void handleJsonMessage(const json& msg);
    bool writeRaw(const std::string& s);
};
