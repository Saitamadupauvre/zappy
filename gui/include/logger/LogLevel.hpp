#pragma once
#include <string_view>

namespace zappy {

enum class LogLevel {
    TRACE,
    INFO,
    DEBUG,
    WARNING,
    ERROR,
    NONE,
    UNKNOWN
};

inline std::string_view logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE:   return "TRACE";
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::NONE:    return "NONE";
        default:                return "UNKNOWN";
    }
}

inline LogLevel stringToLogLevel(std::string_view str) {
    if (str == "trace")   return LogLevel::TRACE;
    if (str == "debug")   return LogLevel::DEBUG;
    if (str == "info")    return LogLevel::INFO;
    if (str == "warn" || str == "warning") return LogLevel::WARNING;
    if (str == "error")   return LogLevel::ERROR;
    if (str == "none")    return LogLevel::NONE;
    
    return LogLevel::UNKNOWN;
}

} // namespace zappy