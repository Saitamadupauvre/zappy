#pragma once
#include "IFlag.hpp"
#include "cli/CliParser.hpp"

namespace zappy {

class ValueFlag : public IFlag
{
    public:
        using ValueCallback = ParseResult (CliParser::*)(const std::string&);

        ValueFlag(std::string shortName, bool mandatory, CliParser* parserInstance, ValueCallback callback);
        ~ValueFlag() override = default;

        [[nodiscard]] bool isMandatory() const noexcept override { return _isMandatory; }
        [[nodiscard]] bool isSet() const noexcept override { return _isSet; }
        [[nodiscard]] const std::string& getShortName() const noexcept override { return _shortName; }

        ParseResult tryParse(std::vector<std::string>::const_iterator& it, 
                      std::vector<std::string>::const_iterator end) override;

    private:
        ParseResult run(const std::string& value);
        
        std::string _shortName;
        bool _isMandatory;
        CliParser* _parser;
        ValueCallback _callback;
        bool _isSet = false;
};

} // namespace zappy