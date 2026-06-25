#include "CommandParser.hpp"
#include <sstream>
#include <iostream>

namespace zappy {

std::unordered_map<std::string, net::MessageKind> CommandParser::_stringToKind;

CommandParser::CommandParser() { initKindMap(); }

void CommandParser::initKindMap() {
    _stringToKind["msz"] = net::MessageKind::Msz;
    _stringToKind["bct"] = net::MessageKind::Bct;
    _stringToKind["tna"] = net::MessageKind::Tna;
    _stringToKind["pnw"] = net::MessageKind::Pnw;
    _stringToKind["ppo"] = net::MessageKind::Ppo;
    _stringToKind["plv"] = net::MessageKind::Plv;
    _stringToKind["pin"] = net::MessageKind::Pin;
    _stringToKind["pex"] = net::MessageKind::Pex;
    _stringToKind["pbc"] = net::MessageKind::Pbc;
    _stringToKind["pic"] = net::MessageKind::Pic;
    _stringToKind["pie"] = net::MessageKind::Pie;
    _stringToKind["pfk"] = net::MessageKind::Pfk;
    _stringToKind["pdr"] = net::MessageKind::Pdr;
    _stringToKind["pgt"] = net::MessageKind::Pgt;
    _stringToKind["pdi"] = net::MessageKind::Pdi;
    _stringToKind["enw"] = net::MessageKind::Enw;
    _stringToKind["ebo"] = net::MessageKind::Ebo;
    _stringToKind["edi"] = net::MessageKind::Edi;
    _stringToKind["sgt"] = net::MessageKind::Sgt;
    _stringToKind["sst"] = net::MessageKind::Sst;
    _stringToKind["seg"] = net::MessageKind::Seg;
    _stringToKind["smg"] = net::MessageKind::Smg;
    _stringToKind["suc"] = net::MessageKind::Suc;
    _stringToKind["sbp"] = net::MessageKind::Sbp;
    _stringToKind["stu"] = net::MessageKind::Stu;
}

net::Message CommandParser::parseLine(const std::string& line) {
    net::Message msg;
    msg.raw = line;
    _log.trace("Parsing line: '", line, "'");

    if (line.empty()) {
        _log.debug("Received an empty line, skipping.");
        return msg;
    }

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (tokenStream >> token) {
        tokens.push_back(token);
    }
    if (tokens.empty()) return msg;

    msg.args = std::vector<std::string>(tokens.begin() + 1, tokens.end());

    auto it = _stringToKind.find(tokens[0]);

    if (it != _stringToKind.end()) {
        msg.kind = it->second;
        _log.debug("Command mapped successfully: ", tokens[0], " (Args count: ", msg.args.size(), ")");
    } else if (tokens.size() == 1 && !tokens[0].empty() &&
               tokens[0].find_first_not_of("0123456789") == std::string::npos) {
        // bare unsigned long: server uptime response to "stu"
        msg.kind = net::MessageKind::Stu;
        msg.args = {tokens[0]};
        _log.debug("Bare number mapped as stu: ", tokens[0]);
    } else {
        msg.kind = net::MessageKind::Unknown;
        _log.warn("Unknown command received from server: '", tokens[0], "'");
    }
    
    return msg;
}

} // namespace zappy