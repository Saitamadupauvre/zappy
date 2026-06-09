#pragma once

#include "parser/Resources/Resources.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

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
};

class World {
public:
    World() = default;
    ~World() = default;

    void resize(int width, int height);
    void setTile(int x, int y, const Resources& resources);
    void addPlayer(PlayerState player);
    void movePlayer(uint32_t id, int x, int y, int orientation);
    void setPlayerLevel(uint32_t id, int level);
    void setPlayerInventory(uint32_t id, const Resources& inv);
    void removePlayer(uint32_t id);
    void addEgg(EggState egg);
    void removeEgg(uint32_t id);
    void addTeam(const std::string& name);

    const Tile& getTile(int x, int y) const;
    const PlayerState* getPlayer(uint32_t id) const;
    const EggState* getEgg(uint32_t id) const;
    const std::unordered_map<uint32_t, PlayerState>& getPlayers() const;
    const std::vector<std::string>& getTeams() const;
    int getWidth() const noexcept { return _width; }
    int getHeight() const noexcept { return _height; }
    bool isReady() const noexcept { return _width > 0 && _height > 0; }

private:
    bool inBounds(int x, int y) const noexcept;

    int _width = 0;
    int _height = 0;
    std::vector<std::vector<Tile>> _grid;
    std::unordered_map<uint32_t, PlayerState> _players;
    std::unordered_map<uint32_t, EggState> _eggs;
    std::vector<std::string> _teams;
};

} // namespace zappy
