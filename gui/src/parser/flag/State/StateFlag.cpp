#include "StateFlag.hpp"

namespace zappy {

StateFlag::StateFlag(std::string shortName, CliParser* parserInstance, StateCallback callback)
    : _shortName(std::move(shortName)), _parser(parserInstance), _callback(callback) {}

ParseResult StateFlag::tryParse(
    std::vector<std::string>::const_iterator& it, 
    [[maybe_unused]] std::vector<std::string>::const_iterator end)
{
    if (*it != _shortName) {
        return ParseResult::NoMatch;
    }
    if (_isSet) {
        throw CliParserException("Flag " + _shortName + " is duplicated.");
    }
    _isSet = true;
    return run();
}

ParseResult StateFlag::run() {
    if (_parser && _callback) {
        return (_parser->*_callback)();
    }
    return ParseResult::Error;
}

} // namespace zappy