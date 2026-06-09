#include "Resources.hpp"
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace zappy {

template <>
Resources parseArg<Resources>(const std::vector<std::string>& args, size_t& index) {
    if (index + 7 > args.size()) {
        throw std::runtime_error("Not enough arguments to parse Resources (needs 7)");
    }
    
    Resources r;
    std::istringstream(args[index++]) >> r.food;
    std::istringstream(args[index++]) >> r.linemate;
    std::istringstream(args[index++]) >> r.deraumere;
    std::istringstream(args[index++]) >> r.sibur;
    std::istringstream(args[index++]) >> r.mendiane;
    std::istringstream(args[index++]) >> r.phiras;
    std::istringstream(args[index++]) >> r.thystame;
    return r;
}

void Resources::dump() const {
    std::cout << "food: " << food << ", linemate: " << linemate 
              << ", deraumere: " << deraumere << ", sibur: " << sibur 
              << ", mendiane: " << mendiane << ", phiras: " << phiras 
              << ", thystame: " << thystame << "\n";
}

} // namespace zappy