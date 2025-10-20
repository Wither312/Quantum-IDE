#pragma once

#include <string_view>

namespace core {

    enum class LogLevel { Info, Warning, Error, Debug };

    class Logger {
    public:
        Logger() {}
        ~Logger(){}

        void log(std::string_view message, LogLevel level = LogLevel::Info) {}

        void setLogLevel(LogLevel level);
        [[nodiscard]] LogLevel getLogLevel() const noexcept;

    private:
        LogLevel m_currentLevel{};
    };

} // namespace core
