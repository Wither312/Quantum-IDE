#include "LSP.hpp"

#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#include <memory>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#endif

LSPClient g_LSPClient;

LSPClient::LSPClient(std::string serverPath, std::vector<std::string> args)
    : serverPath(std::move(serverPath)), args(std::move(args)) {}

LSPClient::~LSPClient() {
    stop();
}

int LSPClient::nextId() {
    std::lock_guard<std::mutex> lock(writeMutex);
    return idCounter++;
}

bool LSPClient::start() {
    if (running) return true;

#ifdef WIN32
    if (!spawnClangdWin()) {
        if (logCB) logCB("Failed to spawn clangd on Windows.");
        return false;
    }
#else
    if (!spawnClangdLinux()) {
        if (logCB) logCB("Failed to spawn clangd on Linux.");
        return false;
    }
#endif

    running = true;
    readerThread = std::thread(&LSPClient::readerLoop, this);

    json initParams = {
        {"processId", (int)getpid()},
        {"rootUri", nullptr},
        {"capabilities", json::object()}
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

#ifdef WIN32
    closeHandlesWin();
#else
    if (toChild_fd != -1) {
        close(toChild_fd);
        toChild_fd = -1;
    }
    if (readerThread.joinable()) readerThread.join();
    if (childPid != -1) {
        int status = 0;
        waitpid(childPid, &status, 0);
        childPid = -1;
    }
#endif
}

bool LSPClient::writeRaw(const std::string& s) {
#ifdef WIN32
    return writeRawWin(s);
#else
    return writeRawLinux(s);
#endif
}

#ifndef WIN32

bool LSPClient::spawnClangdLinux() {
    int inpipe[2];  // parent -> child
    int outpipe[2]; // child -> parent

    if (pipe(inpipe) == -1) return false;
    if (pipe(outpipe) == -1) { close(inpipe[0]); close(inpipe[1]); return false; }

    pid_t pid = fork();
    if (pid < 0) {
        close(inpipe[0]); close(inpipe[1]);
        close(outpipe[0]); close(outpipe[1]);
        return false;
    }

    if (pid == 0) {
        // child
        dup2(inpipe[0], STDIN_FILENO);
        dup2(outpipe[1], STDOUT_FILENO);

        // close fds not needed
        close(inpipe[0]); close(inpipe[1]);
        close(outpipe[0]); close(outpipe[1]);

        // build argv
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(serverPath.c_str()));
        for (auto &a : args) argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        _exit(127);
    }

    // parent
    childPid = pid;
    toChild_fd = inpipe[1];
    fromChild_fd = outpipe[0];

    // close parent unused ends
    close(inpipe[0]);
    close(outpipe[1]);

    // set blocking read (default)
    int flags = fcntl(fromChild_fd, F_GETFL, 0);
    fcntl(fromChild_fd, F_SETFL, flags & ~O_NONBLOCK);

    return true;
}

bool LSPClient::writeRawLinux(const std::string& s) {
    std::lock_guard<std::mutex> lock(writeMutex);
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
#endif

#ifdef WIN32

static std::string utf8FromWString(const std::wstring& w) {
    if (w.empty()) return {};
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), strTo.data(), sizeNeeded, nullptr, nullptr);
    return strTo;
}

bool LSPClient::spawnClangdWin() {
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    HANDLE hChildStd_OUT_Rd = nullptr;
    HANDLE hChildStd_OUT_Wr = nullptr;
    HANDLE hChildStd_IN_Rd = nullptr;
    HANDLE hChildStd_IN_Wr = nullptr;

    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) {
        return false;
    }
    if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(hChildStd_OUT_Wr);
        return false;
    }
    if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) {
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(hChildStd_OUT_Wr);
        return false;
    }
    if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);
        CloseHandle(hChildStd_IN_Wr);
        return false;
    }

    std::string commandLine = "\"" + serverPath + "\"";
    for (auto& arg : args) {
        commandLine += " \"" + arg + "\"";
    }

    int wLen = MultiByteToWideChar(CP_UTF8, 0, commandLine.c_str(), -1, nullptr, 0);
    std::wstring wCommandLine(wLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, commandLine.c_str(), -1, wCommandLine.data(), wLen);

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    STARTUPINFOW siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOW));
    siStartInfo.cb = sizeof(STARTUPINFOW);
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.hStdInput = hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    BOOL bSuccess = CreateProcessW(
        nullptr,
        wCommandLine.data(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &siStartInfo,
        &piProcInfo
    );

    if (!bSuccess) {
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);
        CloseHandle(hChildStd_IN_Wr);
        return false;
    }

    childStd_OUT_Rd = hChildStd_OUT_Rd;
    childStd_IN_Wr = hChildStd_IN_Wr;
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(hChildStd_IN_Rd);

    processHandle = piProcInfo.hProcess;
    
    CloseHandle(piProcInfo.hThread);

    return true;
}

bool LSPClient::writeRawWin(const std::string& s) {
    std::lock_guard<std::mutex> lock(writeMutex);
    if (childStd_IN_Wr == nullptr) return false;

    DWORD totalWritten = 0;
    BOOL bSuccess = WriteFile(
        (HANDLE)childStd_IN_Wr,
        s.c_str(),
        (DWORD)s.size(),
        &totalWritten,
        nullptr
    );
    return bSuccess && totalWritten == s.size();
}

void LSPClient::closeHandlesWin() {
    if (childStd_IN_Wr != nullptr) {
        CloseHandle((HANDLE)childStd_IN_Wr);
        childStd_IN_Wr = nullptr;
    }
    // Close read handle to stop reader thread
    if (readerThread.joinable()) readerThread.join();
    // Wait for process to exit
    if (processHandle) {
        WaitForSingleObject((HANDLE)processHandle, 2000);
        CloseHandle((HANDLE)processHandle);
        processHandle = nullptr;
    }
    // Close other handles
    if (childStd_OUT_Rd) {
        CloseHandle((HANDLE)childStd_OUT_Rd);
        childStd_OUT_Rd = nullptr;
    }
}

#endif

void LSPClient::readerLoop() {
    std::string buffer;
    constexpr size_t bufSize = 8192;
    std::vector<char> readBuf(bufSize);

    while (running) {
#ifdef WIN32
        if (childStd_OUT_Rd == nullptr) break;
        DWORD bytesRead = 0;
        BOOL bSuccess = ReadFile(
            (HANDLE)childStd_OUT_Rd,
            readBuf.data(),
            (DWORD)bufSize,
            &bytesRead,
            nullptr
        );
        if (!bSuccess || bytesRead == 0) {
            break;
        }
        buffer.append(readBuf.data(), readBuf.data() + bytesRead);
    #else
        if (fromChild_fd == -1) break;
        ssize_t bytesRead = read(fromChild_fd, readBuf.data(), bufSize);
        if (bytesRead <= 0) {
            break;
        }
        buffer.append(readBuf.data(), readBuf.data() + bytesRead);
    #endif
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
                    while (i < lenStr.size() && isspace((unsigned char)lenStr[i])) i++;
                    try {
                        contentLength = std::stoul(lenStr.substr(i));
                    } catch (...) {
                        contentLength = 0;
                    }
                }
            }
            size_t totalHeaderSize = pos + 4 + contentLength;
            if (buffer.size() < totalHeaderSize) break;

            std::string content = buffer.substr(pos + 4, contentLength);
            content.erase(0, totalHeaderSize);
            
            try {
                json msg = json::parse(content);
                handleJsonMessage(msg);
            }
            catch (const std::exception& e) {
                if (logCB) logCB(std::string("Failed to parse JSON message: ") + e.what());
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
                std::string uri;
                if (params.contains("uri")) uri = params["uri"].get<std::string>();
                if (diagnosticsCB) {
                    diagnosticsCB(uri, params["diagnostics"]);
                }
            } else {
                if (logCB) logCB("Notification: " + method + " -> " + msg.dump());
            }
        }
        return;
    }
    if (msg.contains("id")) {
        int id = -1;
        try { id = msg["id"].get<int>(); } catch (...) { id = -1; }
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
                    else if (it.contains("textEdit") && it["textEdit"].is_object() && it["textEdit"].contains("newText"))
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
            return;
        }
    }
}

int LSPClient::textDocumentCompletion(const std::string& uri, int line, int character) {
    int id = nextId();
    json params = {
        {"textDocument", {{"uri", uri}}},
        {"position", {{"line", line}, {"character", character}}},
        {"context", json::object()}
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

void LSPClient::textDocumentDidOpen(const std::string& uri, const std::string& languageId, const std::string& text) {
    json params = {
        {"textDocument", {
            {"uri", uri},
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

void LSPClient::textDocumentDidChange(const std::string& uri, const std::string& text) {
    json params = {
        {"textDocument", {
            {"uri", uri},
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