#include "LSP.hpp"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <memory>
static std::string utf8FromWString(const std::wstring& w);
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#endif

namespace fs = std::filesystem;

// ==================== PLATFORM IMPLEMENTATIONS ====================

#ifdef _WIN32
class Win32PlatformImpl : public LSPClient::PlatformImpl {
public:
    HANDLE processHandle = NULL;
    HANDLE childStd_IN_Wr = NULL;
    HANDLE childStd_OUT_Rd = NULL;

    ~Win32PlatformImpl() override {
        closeHandles();
    }

    bool spawnClangd(const std::string& serverPath, const std::vector<std::string>& args) override {
        SECURITY_ATTRIBUTES saAttr{};
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = nullptr;

        HANDLE hChildStd_OUT_Rd = nullptr;
        HANDLE hChildStd_OUT_Wr = nullptr;
        HANDLE hChildStd_IN_Rd = nullptr;
        HANDLE hChildStd_IN_Wr = nullptr;

        // Create stdout pipe
        if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) return false;
        if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
			DWORD err = GetLastError();
			LOG("[spawnClangd]: SetHandleInformation failed: %lu", core::Log::Error, err);
            CloseHandle(hChildStd_OUT_Rd); CloseHandle(hChildStd_OUT_Wr); return false;
        }

        // Create stdin pipe
        if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) {
			DWORD err = GetLastError();
			LOG("[spawnClangd]: CreatePipe failed: %lu", core::Log::Error, err);
            CloseHandle(hChildStd_OUT_Rd); CloseHandle(hChildStd_OUT_Wr); return false;
        }
        if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
            CloseHandle(hChildStd_OUT_Rd); CloseHandle(hChildStd_OUT_Wr);
            CloseHandle(hChildStd_IN_Rd); CloseHandle(hChildStd_IN_Wr); return false;
        }

        // Build command line
        std::string commandLine = "\"" + serverPath + "\"";
        for (const auto& arg : args) {
            commandLine += " \"" + arg + "\"";
        }

        int wLen = MultiByteToWideChar(CP_UTF8, 0, commandLine.c_str(), -1, nullptr, 0);
        if (wLen == 0) {
			LOG("Command line failed to convert to wide string", core::Log::Error);
            CloseHandle(hChildStd_OUT_Rd); CloseHandle(hChildStd_OUT_Wr);
            CloseHandle(hChildStd_IN_Rd); CloseHandle(hChildStd_IN_Wr); return false;
        }

        std::wstring wCommandLine(wLen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, commandLine.c_str(), -1, wCommandLine.data(), wLen);

        PROCESS_INFORMATION piProcInfo{};
        STARTUPINFOW siStartInfo{};
        siStartInfo.cb = sizeof(STARTUPINFOW);
        siStartInfo.hStdError = hChildStd_OUT_Wr;
        siStartInfo.hStdOutput = hChildStd_OUT_Wr;
		siStartInfo.hStdError = hChildStd_OUT_Wr;
        siStartInfo.hStdInput = hChildStd_IN_Rd;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        LOG("Spawning: %s %s", core::Log::Tracer, serverPath.c_str(), commandLine.c_str());

        BOOL bSuccess = CreateProcessW(nullptr, wCommandLine.data(), nullptr, nullptr, TRUE,
                                     CREATE_NO_WINDOW, nullptr, nullptr, &siStartInfo, &piProcInfo);

        if (!bSuccess) {
            DWORD err = GetLastError();
            LOG("[spawnClangd]: CreateProcessW failed: %lu", core::Log::Error, err);
            CloseHandle(hChildStd_OUT_Rd); CloseHandle(hChildStd_OUT_Wr);
            CloseHandle(hChildStd_IN_Rd); CloseHandle(hChildStd_IN_Wr); 
            return false;
        }

        // Store handles
        childStd_OUT_Rd = hChildStd_OUT_Rd;
        childStd_IN_Wr = hChildStd_IN_Wr;
        processHandle = piProcInfo.hProcess;

        // Close unused handles
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);
        CloseHandle(piProcInfo.hThread);

        return true;
    }

    bool writeRaw(const std::string& s) override {
        if (childStd_IN_Wr == nullptr) return false;
        DWORD exitCode = 0;
        GetExitCodeProcess(processHandle, &exitCode);
        if (exitCode != STILL_ACTIVE) return false;

        DWORD totalWritten = 0;
        BOOL bSuccess = WriteFile(childStd_IN_Wr, s.c_str(), (DWORD)s.size(), &totalWritten, nullptr);

        if (!bSuccess) {
            DWORD err = GetLastError();
            LOG("[writeRaw]: WriteFile failed: %lu", core::Log::Error, err);
		}

        return bSuccess && totalWritten == s.size();
    }

    void closeHandles() override {
        if (childStd_IN_Wr != nullptr) {
            CloseHandle(childStd_IN_Wr);
            childStd_IN_Wr = nullptr;
        }
        if (processHandle != NULL) {
            WaitForSingleObject(processHandle, 2000);
            CloseHandle(processHandle);
            processHandle = NULL;
        }
        if (childStd_OUT_Rd != NULL) {
            CloseHandle(childStd_OUT_Rd);
            childStd_OUT_Rd = NULL;
        }
    }

    void* getReadHandle() const override { return childStd_OUT_Rd; }
    bool isProcessAlive() const override {
        if (processHandle == NULL) return false;
        DWORD exitCode = 0;
        GetExitCodeProcess(processHandle, &exitCode);
        return exitCode == STILL_ACTIVE;
    }
};
#endif

#ifndef _WIN32
class LinuxPlatformImpl : public LSPClient::PlatformImpl {
public:
    int toChild_fd = -1;
    int fromChild_fd = -1;
    pid_t childPid = -1;

    ~LinuxPlatformImpl() override {
        closeHandles();
    }

    bool spawnClangd(const std::string& serverPath, const std::vector<std::string>& args) override {
        int inpipe[2], outpipe[2];
        if (pipe(inpipe) == -1 || pipe(outpipe) == -1) {
            if (inpipe[0] != -1) { close(inpipe[0]); close(inpipe[1]); }
            if (outpipe[0] != -1) { close(outpipe[0]); close(outpipe[1]); }
            return false;
        }

        pid_t pid = fork();
        if (pid < 0) {
            close(inpipe[0]); close(inpipe[1]);
            close(outpipe[0]); close(outpipe[1]);
            return false;
        }

        if (pid == 0) { // child
            dup2(inpipe[0], STDIN_FILENO);
            dup2(outpipe[1], STDOUT_FILENO);
            close(inpipe[0]); close(inpipe[1]);
            close(outpipe[0]); close(outpipe[1]);

            std::vector<char*> argv;
            argv.push_back(const_cast<char*>(serverPath.c_str()));
            for (const auto& a : args) argv.push_back(const_cast<char*>(a.c_str()));
            argv.push_back(nullptr);

            execvp(argv[0], argv.data());
            _exit(127);
        }

        // parent
        childPid = pid;
        toChild_fd = inpipe[1];
        fromChild_fd = outpipe[0];
        close(inpipe[0]);
        close(outpipe[1]);

        int flags = fcntl(fromChild_fd, F_GETFL, 0);
        fcntl(fromChild_fd, F_SETFL, flags & ~O_NONBLOCK);

        return true;
    }

    bool writeRaw(const std::string& s) override {
        if (toChild_fd == -1) return false;
        ssize_t total = 0;
        const char* ptr = s.c_str();
        ssize_t remaining = (ssize_t)s.size();
        while (remaining > 0) {
            ssize_t w = write(toChild_fd, ptr + total, remaining);
            if (w <= 0) return false;
            total += w;
            remaining -= w;
        }
        return true;
    }

    void closeHandles() override {
        if (toChild_fd != -1) {
            close(toChild_fd);
            toChild_fd = -1;
        }
        if (childPid != -1) {
            int status = 0;
            waitpid(childPid, &status, 0);
            childPid = -1;
        }
        if (fromChild_fd != -1) {
            close(fromChild_fd);
            fromChild_fd = -1;
        }
    }

    void* getReadHandle() const override { return (void*)(intptr_t)fromChild_fd; }
    bool isProcessAlive() const override { return childPid != -1; }
};
#endif

// ==================== LSPCLIENT IMPLEMENTATION ====================

LSPClient::LSPClient(std::string serverPath, std::vector<std::string> args)
    : serverPath(std::move(serverPath)), args(std::move(args)) {
#ifdef _WIN32
    platform = std::make_unique<Win32PlatformImpl>();
#else
    platform = std::make_unique<LinuxPlatformImpl>();
#endif
}

LSPClient::~LSPClient() {
    stop();
}

int LSPClient::nextId() {
    std::lock_guard<std::mutex> lock(writeMutex);
    return idCounter++;
}

bool LSPClient::start() {
    if (running) return true;

    if (!platform->spawnClangd(serverPath, args)) {
        if (logCB) LOG("Failed to spawn clangd process", core::Log::LogLevel::Error);
        return false;
    }

    running = true;
    readerThread = std::thread(&LSPClient::readerLoop, this);

    // Initialize LSP
    json initParams = {
    {"processId", (int)getpid()},
    {"rootUri", toLspUri(fs::current_path())},
    {"capabilities", {
        {"textDocument", {
            {"completion", {
                {"completionItem", {
                    {"snippetSupport", true}
                }}
            }}
        }}
    }}
};

    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", nextId()},
        {"method", "initialize"},
        {"params", initParams}
    };

    std::string initStr = initRequest.dump();
    std::ostringstream header;
    header << "Content-Length: " << initStr.size() << "\r\n\r\n" << initStr;
    writeRaw(header.str());

    json initNotif = {
        {"jsonrpc", "2.0"},
        {"method", "initialized"},
        {"params", json::object()}
    };

    std::string notifStr = initNotif.dump();
    std::ostringstream notifHeader;
    notifHeader << "Content-Length: " << notifStr.size() << "\r\n\r\n" << notifStr;
    writeRaw(notifHeader.str());

    return true;
}

void LSPClient::stop() {
    if (!running) return;
    running = false;

    json shutdownRequest = {
        {"jsonrpc", "2.0"},
        {"id", nextId()},
        {"method", "shutdown"},
        {"params", json::object()}
    };

    std::string shutdownStr = shutdownRequest.dump();
    std::ostringstream header;
    header << "Content-Length: " << shutdownStr.size() << "\r\n\r\n" << shutdownStr;
    writeRaw(header.str());

    json exitNotif = {
        {"jsonrpc", "2.0"},
        {"method", "exit"},
        {"params", json::object()}
    };

    std::string exitStr = exitNotif.dump();
    std::ostringstream exitHeader;
    exitHeader << "Content-Length: " << exitStr.size() << "\r\n\r\n" << exitStr;
    writeRaw(exitHeader.str());

    platform->closeHandles();

    if (readerThread.joinable()) {
        readerThread.join();
    }
}

bool LSPClient::writeRaw(const std::string& s) {
    return platform->writeRaw(s);
}

void LSPClient::readerLoop() {
    std::string buffer;
    constexpr size_t bufSize = 8192;
    std::vector<char> readBuf(bufSize);

    while (running) {
        void* readHandle = platform->getReadHandle();
        if (!readHandle) break;

#ifdef _WIN32
        HANDLE h = (HANDLE)readHandle;
        if (!platform->isProcessAlive()) break;
        DWORD bytesRead = 0;
        BOOL bSuccess = ReadFile(h, readBuf.data(), static_cast<DWORD>(bufSize), &bytesRead, nullptr);
        if (!bSuccess || bytesRead == 0) break;
#else
        int fd = (int)(intptr_t)readHandle;
        if (!platform->isProcessAlive()) break;
        ssize_t bytesRead = read(fd, readBuf.data(), bufSize);
        if (bytesRead <= 0) break;
#endif

        buffer.append(readBuf.data(), bytesRead);

        while (true) {
            size_t pos = buffer.find("\r\n\r\n");
            if (pos == std::string::npos) break;

            std::istringstream headerStream(buffer.substr(0, pos));
            std::string line;
            size_t contentLength = 0;

            while (std::getline(headerStream, line)) {
                const std::string clPrefix = "Content-Length: ";
                if (line.rfind(clPrefix, 0) == 0) {
                    std::string lenStr = line.substr(clPrefix.size());
                    size_t i = 0;
                    while (i < lenStr.size() && isspace(static_cast<unsigned char>(lenStr[i]))) i++;
                    try {
                        contentLength = std::stoul(lenStr.substr(i));
                    } catch (...) {
                        contentLength = 0;
                    }
                }
            }

            size_t totalMessageSize = pos + 4 + contentLength;
            if (buffer.size() < totalMessageSize) break;

            std::string content = buffer.substr(pos + 4, contentLength);
            buffer.erase(0, totalMessageSize);

            if (!content.empty()) {
                try {
                    json msg = json::parse(content);
                    handleJsonMessage(msg);
                } catch (const std::exception& e) {
                    if (logCB) LOG("Failed to parse JSON: %s", core::Log::LogLevel::Error, e.what());
                }
            }
        }
    }
    running = false;
}

void LSPClient::handleJsonMessage(const json& msg) {
    if (msg.contains("method")) {
        std::string method = msg["method"];
        if (method == "textDocument/publishDiagnostics") {
            if (msg.contains("params")) {
                const auto& params = msg["params"];
                std::string uriStr;
                if (params.contains("uri")) uriStr = params["uri"].get<std::string>();
                fs::path uri = uriStr;
                if (diagnosticsCB) diagnosticsCB(uri, params["diagnostics"]);
            }
        }
        return;
    }

    if (msg.contains("id")) {
        int id = -1;
        try { id = msg["id"].get<int>(); } catch (...) { return; }
        if (id == -1) return;

        if (msg.contains("result")) {
            const auto& res = msg["result"];
            std::vector<CompletionItem> items;
            if (res.is_object() && res.contains("items") && res["items"].is_array()) {
                for (const auto& it : res["items"]) {
                    CompletionItem ci;
                    if (it.contains("label")) ci.label = it["label"].get<std::string>();
                    if (it.contains("detail")) ci.detail = it["detail"].get<std::string>();
                    if (it.contains("insertText")) ci.insertText = it["insertText"].get<std::string>();
                    else if (it.contains("textEdit") && it["textEdit"].contains("newText"))
                        ci.insertText = it["textEdit"]["newText"].get<std::string>();
                    items.push_back(std::move(ci));
                }
            } else if (res.is_array()) {
                for (const auto& it : res) {
                    CompletionItem ci;
                    if (it.contains("label")) ci.label = it["label"].get<std::string>();
                    if (it.contains("detail")) ci.detail = it["detail"].get<std::string>();
                    if (it.contains("insertText")) ci.insertText = it["insertText"].get<std::string>();
                    items.push_back(std::move(ci));
                }
            }
            if (completionCB) completionCB(id, items);
        }
    }
}

std::string LSPClient::toLspUri(const std::filesystem::path& path) const  {
#ifdef _WIN32
    return "file:///" + path.string();
#else
    return "file://" + path.string();
#endif
}

int LSPClient::textDocumentCompletion(const fs::path& uri, int line, int character) {
    int id = nextId();
    json params = {
        {"textDocument", {{"uri", toLspUri(uri)}}},
        {"position", {{"line", line}, {"character", character}}},
        {"context", {
            {"triggerKind", 1}
        }}
    };
    json request = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"method", "textDocument/completion"},
        {"params", params}
    };
    std::string reqStr = request.dump();
    std::ostringstream header;
    header << "Content-Length: " << reqStr.size() << "\r\n\r\n" << reqStr;
    writeRaw(header.str());
    return id;
}

void LSPClient::textDocumentDidOpen(const fs::path& uri, const std::string& languageId, const std::string& text) {
    json params = {
        {"textDocument", {
            {"uri", toLspUri(uri)},
            {"languageId", languageId},
            {"version", 1},
            {"text", text}
        }}
    };
    json notif = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", params}
    };
    std::string notifStr = notif.dump();
    std::ostringstream header;
    header << "Content-Length: " << notifStr.size() << "\r\n\r\n" << notifStr;
    writeRaw(header.str());
}

void LSPClient::textDocumentDidChange(const fs::path& uri, const std::string& text) {
    json params = {
        {"textDocument", {
            {"uri", toLspUri(uri)},
            {"version", 2}
        }},
        {"contentChanges", json::array({
            json::object({
                {"text", text}
            })
        })}
    };
    json notif = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", params}
    };
    std::string notifStr = notif.dump();
    std::ostringstream header;
    header << "Content-Length: " << notifStr.size() << "\r\n\r\n" << notifStr;
    writeRaw(header.str());
}
