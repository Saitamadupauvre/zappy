#pragma once

#include "world/WorldTypes.hpp"
#include "event/WorldEvent.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

namespace zappy {

class World {
public:
    World() = default;
    ~World() = default;

    void setEventDispatcher(std::function<void(const event::WorldEvent&)> dispatch);

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
    void emit(event::WorldEvent ev) { dispatch(std::move(ev)); }

    const Tile& getTile(int x, int y) const;
    const PlayerState* getPlayer(uint32_t id) const;
    const EggState* getEgg(uint32_t id) const;
    const std::unordered_map<uint32_t, PlayerState>& getPlayers() const;
    const std::unordered_map<uint32_t, EggState>& getEggs() const { return _eggs; }
    const std::vector<std::string>& getTeams() const;
    int getWidth() const noexcept    { return _width; }
    int getHeight() const noexcept   { return _height; }
    int getTimeUnit() const noexcept { return _timeUnit; }
    bool isReady() const noexcept    { return _width > 0 && _height > 0; }
    void setTimeUnit(int t)          { _timeUnit = t > 0 ? t : 1; }

private:
    bool inBounds(int x, int y) const noexcept;
    void dispatch(event::WorldEvent ev) const;

    int _width    = 0;
    int _height   = 0;
    int _timeUnit = 100;
    std::vector<std::vector<Tile>> _grid;
    std::unordered_map<uint32_t, PlayerState> _players;
    std::unordered_map<uint32_t, EggState> _eggs;
    std::vector<std::string> _teams;
    std::function<void(const event::WorldEvent&)> _dispatch;
};

} // namespace zappy
