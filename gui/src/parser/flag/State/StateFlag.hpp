#pragma once 

#include "IFlag.hpp"
#include "cli/CliParser.hpp"

namespace zappy {

class StateFlag : public IFlag
{
    public:
        using StateCallback = ParseResult (CliParser::*)() const;

        StateFlag(std::string shortName, CliParser* parserInstance, StateCallback callback);

        bool isMandatory() const noexcept override { return false; }
        bool isSet() const noexcept override { return _isSet; }

        virtual const std::string& getShortName() const override { return _shortName; }

        ParseResult tryParse(std::vector<std::string>::const_iterator& it, 
                      std::vector<std::string>::const_iterator end) override;

    private:
        ParseResult run();
        std::string _shortName;
        CliParser* _parser;
        StateCallback _callback;
        bool _isSet = false;
};

} // namespace zappy
