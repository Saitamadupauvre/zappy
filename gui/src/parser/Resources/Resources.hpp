#pragma once

#include <vector>
#include <string>

namespace zappy {

struct Resources {
    int food = 0;
    int linemate = 0;
    int deraumere = 0;
    int sibur = 0;
    int mendiane = 0;
    int phiras = 0;
    int thystame = 0;

    void dump() const;
};

template <typename T>
T parseArg(const std::vector<std::string>& args, size_t& index);

template <>
Resources parseArg<Resources>(const std::vector<std::string>& args, size_t& index);

} // namespace zappy