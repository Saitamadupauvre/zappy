#pragma once

#include "network/GuiProtocol.hpp"
#include "logger/ContextLogger.hpp"
#include <unordered_map>
#include <string>

namespace zappy {

class CommandParser
{
    public:
        CommandParser();
        ~CommandParser() = default;

        net::Message parseLine(const std::string& line);

    private:
        static std::unordered_map<std::string, net::MessageKind> _stringToKind;
        void initKindMap();
        ContextLogger _log{"CommandParser"};
};

} // namespace zappy