#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <memory>
#include <filesystem>

#include "Log.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct CompletionItem {
    std::string label;
    std::string detail;
    std::string insertText;
};

class LSPClient {
public:
    using OnDiagnostics = std::function<void(const std::filesystem::path& uri, const json& diagnostics)>;
    using OnCompletion = std::function<void(int id, const std::vector<CompletionItem>& items)>;
    using OnLog = std::function<void(const std::string& message)>;

    LSPClient(std::string serverPath = "clangd", std::vector<std::string> args = {});
    ~LSPClient();

    // Lifecycle
    bool start();
    void stop();

    // Requests
    int textDocumentCompletion(const std::filesystem::path& uri, int line, int character);
    void textDocumentDidOpen(const std::filesystem::path& uri, const std::string& languageId, const std::string& text);
    void textDocumentDidChange(const std::filesystem::path& uri, const std::string& text);

    // Configuration
    void setOnDiagnostics(OnDiagnostics cb) { diagnosticsCB = std::move(cb); }
    void setOnCompletion(OnCompletion cb) { completionCB = std::move(cb); }
    void setOnLog(OnLog cb) { logCB = std::move(cb); }

    // Platform abstraction
    class PlatformImpl {
    public:
        virtual ~PlatformImpl() = default;
        virtual bool spawnClangd(const std::string& serverPath, const std::vector<std::string>& args) = 0;
        virtual bool writeRaw(const std::string& s) = 0;
        virtual void closeHandles() = 0;
        virtual void* getReadHandle() const = 0;  // HANDLE or int fd
        virtual bool isProcessAlive() const = 0;
    };

    std::unique_ptr<PlatformImpl> platform;

private:
    std::string serverPath;
    std::vector<std::string> args;

    std::thread readerThread;
    std::atomic<bool> running{false};
    std::mutex writeMutex;
    int idCounter = 1;
    std::unordered_map<int, int> pendingRequests;

    // Callbacks
    OnDiagnostics diagnosticsCB;
    OnCompletion completionCB;
    OnLog logCB;

    // Core methods
    int nextId();
    bool writeRaw(const std::string& s);
    void readerLoop();
    void handleJsonMessage(const json& msg);
};
