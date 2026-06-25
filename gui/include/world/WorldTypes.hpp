#pragma once

#include "parser/Resources/Resources.hpp"
#include <cstdint>
#include <string>

namespace zappy {

struct Tile {
    int x = 0;
    int y = 0;
    Resources resources;
};

struct PlayerState {
    uint32_t id = 0;
    int x = 0;
    int y = 0;
    int orientation = 0;
    int level = 1;
    std::string team;
    Resources inventory;
};

struct EggState {
    uint32_t id = 0;
    uint32_t playerId = 0;
    int x = 0;
    int y = 0;
    std::string team;
};

} // namespace zappy
