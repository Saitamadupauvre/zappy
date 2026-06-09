#include "CliParser.hpp"
#include "flag/Value/ValueFlag.hpp"
#include "flag/State/StateFlag.hpp"
#include <iostream>
#include <algorithm>

namespace zappy {

CliParser::CliParser()
{
    _flags.push_back(std::make_unique<ValueFlag>("-p", true, this, &CliParser::parsePort));     
    _flags.push_back(std::make_unique<ValueFlag>("-h", false, this, &CliParser::parseMachine));
    _flags.push_back(std::make_unique<ValueFlag>("-v", false, this, &CliParser::parseVerbose));
    _flags.push_back(std::make_unique<ValueFlag>("--verbose", false, this, &CliParser::parseVerbose));
    _flags.push_back(std::make_unique<ValueFlag>("--log-file", false, this, &CliParser::parseLogFile));
    _flags.push_back(std::make_unique<ValueFlag>("--log-file-level", false, this, &CliParser::parseLogFileLevel));
    _flags.push_back(std::make_unique<StateFlag>("--help", this, &CliParser::printUsage));

}

ParseResult CliParser::parsePort(const std::string& arg)
{
    _config.port = arg;
    return ParseResult::Matched;
}

ParseResult CliParser::parseMachine(const std::string& arg)
{
    _config.machine = arg;
    return ParseResult::Matched;
}

ParseResult CliParser::parseVerbose(const std::string& arg)
{
    std::string lowerArg = arg;
    std::transform(lowerArg.begin(), lowerArg.end(), lowerArg.begin(), ::tolower);

    _config.consoleLog.level = stringToLogLevel(lowerArg);

    if (_config.consoleLog.level == LogLevel::UNKNOWN) {
        throw CliParserException("Invalid verbose level: '" + arg + "'. Expected: trace, debug, info, warn, error, none.");
    }

    return ParseResult::Matched;
}

ParseResult CliParser::parseLogFile(const std::string& arg)
{
    if (arg.empty() || arg[0] == '-') {
        throw CliParserException("Invalid file path for --log-file flag.");
    }
    _config.fileLog.filePath = arg;
    if (_config.fileLog.level == LogLevel::NONE) {
        _config.fileLog.level = LogLevel::TRACE;
    }
    return ParseResult::Matched;
}

ParseResult CliParser::parseLogFileLevel(const std::string& arg)
{
    std::string lowerArg = arg;
    std::transform(lowerArg.begin(), lowerArg.end(), lowerArg.begin(), ::tolower);

    _config.fileLog.level = stringToLogLevel(lowerArg);

    if (_config.fileLog.level == LogLevel::UNKNOWN) {
        throw CliParserException("Invalid log level: '" + arg + "'. Expected: trace, debug, info, warn, error, none.");
    }

    return ParseResult::Matched;
}

ParseResult CliParser::printUsage() const
{
    std::cout << "USAGE: " << _progName << " -p port [-h machine] [-v level] [--log-file filename] [--log-file-level level]\n\n"
              << "option\t\tdescription\n"
              << "-p, --port port\t\tport number (Mandatory)\n" 
              << "-h, --host machine\thostname of the server (default: localhost)\n"
              << "-v, --verbose level\tset console log level (trace, debug, info, warn, error, none) (default: none)\n"
              << "--log-file filename\tenable logging to a specific file with full TRACE level (default: disabled)\n"
              << "--log-file-level level\toverride the specific file log level (default: trace)\n";
    return ParseResult::Exit;
}

bool CliParser::parseArguments(int argc, const char **argv)
{
    _progName = argv[0];
    const std::vector<std::string> args(argv + 1, argv + argc);

    _log.info("Starting to parse ", args.size(), " command-line arguments.");

    auto it = args.begin();
    
    while (it != args.end()) {
        _log.trace("Processing token: '", *it, "'");
        
        ParseResult currentResult = ParseResult::NoMatch;

        for (const auto& flag : _flags) {
            currentResult = flag->tryParse(it, args.end());
            if (currentResult != ParseResult::NoMatch) {
                _log.debug("Token '", *it, "' matched flag: ", flag->getShortName());
                break;
            }
        }

        if (currentResult == ParseResult::NoMatch) {
            _log.warn("No matching flag found for token: '", *it, "'");
        }

        bool shouldContinue = _actionTable.at(currentResult)(*it);
        if (!shouldContinue) {
            _log.error("Parsing aborted by action table rule on token: '", *it, "'");
            return false;
        }

        ++it;
    }

    std::for_each(_flags.begin(), _flags.end(), [this](const auto& flag) {
        if (flag->isMandatory() && !flag->isSet()) {
            _log.error("Validation failed: Mandatory flag '", flag->getShortName(), "' is missing.");
            throw CliParserException("Missing mandatory argument: " + flag->getShortName());
        }
    });

    if (_config.port.empty()) {
        _log.error("Validation failed: Mandatory port is missing.");
        throw CliParserException("Missing mandatory argument: -p or --port");
    }

    _log.debug("All arguments successfully validated.");
    return true;
}

} // namespace zappy