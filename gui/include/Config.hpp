#pragma once
#include <string>
#include "logger/LogLevel.hpp"

// Ces macros peuvent être surchargées à la compilation via CMake
#ifndef DEFAULT_CONSOLE_LOG_LEVEL
#define DEFAULT_CONSOLE_LOG_LEVEL zappy::LogLevel::NONE
#endif

#ifndef DEFAULT_FILE_LOG_LEVEL
#define DEFAULT_FILE_LOG_LEVEL zappy::LogLevel::NONE
#endif

#ifndef DEFAULT_LOG_FILE_PATH
#define DEFAULT_LOG_FILE_PATH ""
#endif

namespace zappy {

struct LogSinkConfig {
    LogLevel level = LogLevel::NONE;
    std::string filePath = "";
};

struct AppConfig {
    std::string port = "";
    std::string machine = "localhost";

    LogSinkConfig consoleLog{DEFAULT_CONSOLE_LOG_LEVEL, ""};
    LogSinkConfig fileLog{DEFAULT_FILE_LOG_LEVEL, DEFAULT_LOG_FILE_PATH};
};

} // namespace zappy
