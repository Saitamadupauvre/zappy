#pragma once

#include "parser/ICommand.hpp"
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <sstream>

namespace zappy {

class Command : public ICommand {
    public:
        using CommandCallback = std::function<void(const std::vector<std::string>&)>;

        Command(std::string name, size_t minArgs, CommandCallback callback);
        ~Command() override = default;

        void execute(const std::vector<std::string>& args) const override;

    private:
        std::string _name;
        size_t _minArgs;
        CommandCallback _callback;
};

template <typename T>
T parseArg(const std::string& str) {
    if constexpr (std::is_same_v<T, std::string>) {
        return str;
    } else {
        std::istringstream iss(str);
        T value;
        if (!(iss >> value)) {
            throw std::runtime_error("Invalid argument type conversion for: " + str);
        }
        return value;
    }
}

} // namespace zappy