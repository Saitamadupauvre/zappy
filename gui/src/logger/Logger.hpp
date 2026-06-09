#pragma once

#include "logger/LogLevel.hpp"
#include "logger/ISink.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <sstream>
#include <atomic>

namespace zappy {

class Logger
{
    public:
        Logger() = default;
        ~Logger() = default;


        void addSink(std::shared_ptr<ISink> sink, LogLevel minLevel = LogLevel::TRACE);
        void log(LogLevel level, std::string_view origin, std::string_view message);
        void setMinLevel(LogLevel level);

        template <typename... Args>
        void trace(Args&&... args) { log(LogLevel::TRACE, "", buildMessage(std::forward<Args>(args)...)); }
        
        template <typename... Args>
        void debug(Args&&... args) { log(LogLevel::DEBUG, "", buildMessage(std::forward<Args>(args)...)); }
        
        template <typename... Args>
        void info(Args&&... args) { log(LogLevel::INFO, "", buildMessage(std::forward<Args>(args)...)); }
        
        template <typename... Args>
        void warn(Args&&... args) { log(LogLevel::WARNING, "", buildMessage(std::forward<Args>(args)...)); }
        
        template <typename... Args>
        void error(Args&&... args) { log(LogLevel::ERROR, "", buildMessage(std::forward<Args>(args)...)); }

        template <typename... Args>
        void traceWithContext(std::string_view origin, Args&&... args) { log(LogLevel::TRACE, origin, buildMessage(std::forward<Args>(args)...)); }
        
        template <typename... Args>
        void debugWithContext(std::string_view origin, Args&&... args) { log(LogLevel::DEBUG, origin, buildMessage(std::forward<Args>(args)...)); }
        
        template <typename... Args>
        void infoWithContext(std::string_view origin, Args&&... args) { log(LogLevel::INFO, origin, buildMessage(std::forward<Args>(args)...)); }
        
        template <typename... Args>
        void warnWithContext(std::string_view origin, Args&&... args) { log(LogLevel::WARNING, origin, buildMessage(std::forward<Args>(args)...)); }
        
        template <typename... Args>
        void errorWithContext(std::string_view origin, Args&&... args) { log(LogLevel::ERROR, origin, buildMessage(std::forward<Args>(args)...)); }

    private:
        struct SinkConfig {
            std::shared_ptr<ISink> sink;
            LogLevel minLevel;
        };

        std::atomic<LogLevel> _globalMinLevel{LogLevel::NONE};
        std::vector<SinkConfig> _sinks;
        void recalculateGlobalMinLevel();

        template <typename... Args>
        std::string buildMessage(Args&&... args) {
            std::ostringstream oss;
            (oss << ... << std::forward<Args>(args));
            return oss.str();
        }
};

} // namespace zappy