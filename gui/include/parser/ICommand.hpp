#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace zappy {

class ICommand
{
    public:
        virtual ~ICommand() = default;
        virtual void execute(const std::vector<std::string>& args) const = 0;
};

} // namespace zappy