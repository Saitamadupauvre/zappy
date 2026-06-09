#pragma once

#include <string>
#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>
#include "parser/IFlag.hpp"
#include "logger/ContextLogger.hpp"
#include "logger/LogLevel.hpp"
#include "Config.hpp"

namespace zappy {

class CliParserException : public std::runtime_error
{
    public:
        explicit CliParserException(const std::string& msg) : std::runtime_error(msg) {}
};

class CliParser 
{
    public:
        CliParser();
        ~CliParser() = default;

        bool parseArguments(int argc, const char **argv);

        [[nodiscard]] const AppConfig& getConfig() const { return _config; }

    private:
        ContextLogger _log{"CliParser"};

        std::vector<std::unique_ptr<IFlag>> _flags;
        
        AppConfig _config;
        std::string _progName;

        ParseResult parsePort(const std::string& arg);
        ParseResult parseMachine(const std::string& arg);
        
        ParseResult parseVerbose(const std::string& arg);
        ParseResult parseLogFile(const std::string& arg);
        ParseResult parseLogFileLevel(const std::string& arg);
        
        ParseResult printUsage() const;

        using ActionFunction = std::function<bool(const std::string&)>;
        using ActionTable = std::unordered_map<ParseResult, ActionFunction>;

        static inline const ActionTable _actionTable = {
            std::pair{ ParseResult::NoMatch, [](const std::string& arg) -> bool { 
                throw CliParserException("Unknown argument: " + arg); 
            }},
            std::pair{ ParseResult::Error, [](const std::string& arg) -> bool { 
                throw CliParserException("Internal error: Failed to execute callback for flag " + arg); 
            }},
            std::pair{ ParseResult::Exit, [](const std::string&) -> bool { 
                return false; 
            }},
            std::pair{ ParseResult::Matched, [](const std::string&) -> bool { 
                return true;  
            }}
        };
};

} // namespace zappy