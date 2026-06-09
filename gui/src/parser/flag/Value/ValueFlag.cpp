#include "ValueFlag.hpp"

namespace zappy {

ValueFlag::ValueFlag(std::string shortName, bool mandatory, CliParser* parserInstance, ValueCallback callback)
    : _shortName(std::move(shortName)), _isMandatory(mandatory), _parser(parserInstance), _callback(callback) {}

ParseResult ValueFlag::tryParse(
    std::vector<std::string>::const_iterator& it,
    std::vector<std::string>::const_iterator end) 
{
    if (*it != _shortName) {
        return ParseResult::NoMatch;
    }
    if (_isSet) {
        throw CliParserException("Flag " + _shortName + " is duplicated.");
    }
    if (it + 1 == end) {
        throw CliParserException("Option " + _shortName + " requires an argument.");
    }

    std::string value = *(it + 1);

    if (!value.empty() && value[0] == '-') {
        throw CliParserException("Option " + _shortName + " expects a value, but found flag: " + value);
    }

    _isSet = true;

    it++; 
    return run(value);
}

ParseResult ValueFlag::run(const std::string& value) 
{
    if (_parser && _callback) {
        return (_parser->*_callback)(value);
    }
    return ParseResult::Error;
}

} // namespace zappy