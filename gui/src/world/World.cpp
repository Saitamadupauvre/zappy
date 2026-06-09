#include "World.hpp"

namespace zappy {

void World::resize(int width, int height)
{
    _width = width;
    _height = height;
    _grid.assign(height, std::vector<Tile>(width));
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            _grid[y][x] = {x, y, {}};
}

void World::setTile(int x, int y, const Resources& resources)
{
    if (!inBounds(x, y)) return;
    _grid[y][x].resources = resources;
}

void World::addPlayer(PlayerState player)
{
    _players[player.id] = std::move(player);
}

void World::movePlayer(uint32_t id, int x, int y, int orientation)
{
    auto it = _players.find(id);
    if (it == _players.end()) return;
    it->second.x = x;
    it->second.y = y;
    it->second.orientation = orientation;
}

void World::setPlayerLevel(uint32_t id, int level)
{
    auto it = _players.find(id);
    if (it == _players.end()) return;
    it->second.level = level;
}

void World::setPlayerInventory(uint32_t id, const Resources& inv)
{
    auto it = _players.find(id);
    if (it == _players.end()) return;
    it->second.inventory = inv;
}

void World::removePlayer(uint32_t id)
{
    _players.erase(id);
}

void World::addEgg(EggState egg)
{
    _eggs[egg.id] = egg;
}

void World::removeEgg(uint32_t id)
{
    _eggs.erase(id);
}

void World::addTeam(const std::string& name)
{
    _teams.push_back(name);
}

const Tile& World::getTile(int x, int y) const
{
    static const Tile empty{};
    if (!inBounds(x, y)) return empty;
    return _grid[y][x];
}

const PlayerState* World::getPlayer(uint32_t id) const
{
    auto it = _players.find(id);
    return it != _players.end() ? &it->second : nullptr;
}

const EggState* World::getEgg(uint32_t id) const
{
    auto it = _eggs.find(id);
    return it != _eggs.end() ? &it->second : nullptr;
}

const std::unordered_map<uint32_t, PlayerState>& World::getPlayers() const
{
    return _players;
}

const std::vector<std::string>& World::getTeams() const
{
    return _teams;
}

bool World::inBounds(int x, int y) const noexcept
{
    return x >= 0 && y >= 0 && x < _width && y < _height;
}

} // namespace zappy
