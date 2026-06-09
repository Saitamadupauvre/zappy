#include "Command.hpp"
#include <iostream>

namespace zappy {

Command::Command(std::string name, size_t minArgs, CommandCallback callback)
    : _name(std::move(name)), _minArgs(minArgs), _callback(std::move(callback)) 
{
}

void Command::execute(const std::vector<std::string>& args) const 
{
    if (args.size() < _minArgs) {
        std::cerr << "[PROTOCOL ERROR] Command '" << _name 
                  << "' expects at least " << _minArgs 
                  << " arguments, but got " << args.size() << ".\n";
        return;
    }
    
    if (_callback) {
        _callback(args);
    }
}

} // namespace zappy