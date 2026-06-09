#include "logger/Logger.hpp"
#include <chrono>
#include <iomanip>

namespace zappy {

void Logger::addSink(std::shared_ptr<ISink> sink, LogLevel minLevel) {
    _sinks.push_back({sink, minLevel});
    recalculateGlobalMinLevel();
}

void Logger::setMinLevel(LogLevel level) {
    for (auto& config : _sinks) {
        config.minLevel = level;
    }
    recalculateGlobalMinLevel();
}

void Logger::recalculateGlobalMinLevel() {
    LogLevel min = LogLevel::NONE;
    for (const auto& config : _sinks) {
        if (config.minLevel < min) {
            min = config.minLevel;
        }
    }
    _globalMinLevel.store(min);
}

void Logger::log(LogLevel level, std::string_view origin, std::string_view message) {
    if (_globalMinLevel.load() == LogLevel::NONE || level < _globalMinLevel.load()) return;

    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timeT = std::chrono::system_clock::to_time_t(now);
    struct tm timeInfo;
    localtime_r(&timeT, &timeInfo);

    std::ostringstream oss;
    oss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << '.' 
        << std::setfill('0') << std::setw(3) << ms.count() << "] "
        << "[" << logLevelToString(level) << "] ";

    if (!origin.empty()) {
        oss << "[" << origin << "] ";
    }

    oss << message << "\n";

    for (const auto& config : _sinks) {
        if (config.minLevel != LogLevel::NONE && level >= config.minLevel) {
            config.sink->write(level, oss.str());
        }
    }
}

} // namespace zappy