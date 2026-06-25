#include "World.hpp"

namespace zappy {

void World::setEventDispatcher(std::function<void(const event::WorldEvent&)> dispatch)
{
    _dispatch = std::move(dispatch);
}

void World::dispatch(event::WorldEvent ev) const
{
    if (_dispatch) _dispatch(ev);
}

void World::resize(int width, int height)
{
    _width = width;
    _height = height;
    _grid.assign(height, std::vector<Tile>(width));
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            _grid[y][x] = {x, y, {}};
    dispatch(event::WorldResizedEvent{width, height});
}

void World::setTile(int x, int y, const Resources& resources)
{
    if (!inBounds(x, y)) return;
    _grid[y][x].resources = resources;
    dispatch(event::TileChangedEvent{x, y, resources});
}

void World::addPlayer(PlayerState player)
{
    _players[player.id] = player;
    dispatch(event::PlayerAddedEvent{std::move(player)});
}

void World::movePlayer(uint32_t id, int x, int y, int orientation)
{
    auto it = _players.find(id);
    if (it == _players.end()) return;
    it->second.x = x;
    it->second.y = y;
    it->second.orientation = orientation;
    dispatch(event::PlayerMovedEvent{id, x, y, orientation});
}

void World::setPlayerLevel(uint32_t id, int level)
{
    auto it = _players.find(id);
    if (it == _players.end()) return;
    it->second.level = level;
    dispatch(event::PlayerLevelChangedEvent{id, level});
}

void World::setPlayerInventory(uint32_t id, const Resources& inv)
{
    auto it = _players.find(id);
    if (it == _players.end()) return;
    it->second.inventory = inv;
    dispatch(event::PlayerInventoryChangedEvent{id, inv});
}

void World::removePlayer(uint32_t id)
{
    _players.erase(id);
    dispatch(event::PlayerRemovedEvent{id});
}

void World::addEgg(EggState egg)
{
    _eggs[egg.id] = egg;
    dispatch(event::EggAddedEvent{std::move(egg)});
}

void World::removeEgg(uint32_t id)
{
    _eggs.erase(id);
    dispatch(event::EggRemovedEvent{id});
}

void World::addTeam(const std::string& name)
{
    _teams.push_back(name);
    dispatch(event::TeamAddedEvent{name});
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
