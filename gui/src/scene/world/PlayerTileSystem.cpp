#include "PlayerTileSystem.hpp"
#include "event/Event.hpp"
#include "scene/layout/IMapLayout.hpp"
#include <algorithm>
#include <cmath>

namespace zappy {

PlayerTileSystem::PlayerTileSystem(EntityManager& entities, TileMap& tileMap,
                                    AnimationClock& clock)
    : _entities(entities), _tileMap(tileMap), _clock(clock)
{}

void PlayerTileSystem::setActiveLayout(IMapLayout* layout)
{
    _activeLayout = layout;
}

void PlayerTileSystem::onPlayerAdded(uint32_t id, int x, int y, int orientation)
{
    _playerTiles[id]  = {x, y};
    _playerOrient[id] = orientation;

    sendRotateEvent(id, orientationToYaw(orientation), 0.0f,
                    _tileMap.tileUp(x, y), _tileMap.tileForward(x, y));
    restackTile(x, y, 0.0f);
}

void PlayerTileSystem::onPlayerMoved(uint32_t id, int x, int y, int orientation)
{
    bool wrap = false;
    std::pair<int, int> oldTile{-1, -1};
    bool hadTile = false;
    auto it = _playerTiles.find(id);
    if (it != _playerTiles.end()) {
        wrap    = _tileMap.isWrapMove(it->second.first, it->second.second, x, y);
        oldTile = it->second;
        hadTile = true;
    }

    _playerTiles[id]  = {x, y};
    _playerOrient[id] = orientation;

    bool teleport = (_activeLayout && !_activeLayout->animatesWrap() && wrap);

    float visualDur = std::max(_clock.moveDuration(), MIN_MOVE_DURATION);
    float moveDur   = (!_tileMap.built() || teleport) ? 0.0f : visualDur;
    float rotateDur = (!_tileMap.built() || teleport) ? 0.0f : visualDur * 0.5f;

    sendRotateEvent(id, orientationToYaw(orientation), rotateDur,
                    _tileMap.tileUp(x, y), _tileMap.tileForward(x, y));

    restackTile(x, y, moveDur);
    if (hadTile && oldTile != std::make_pair(x, y))
        restackTile(oldTile.first, oldTile.second, moveDur);
}

void PlayerTileSystem::onPlayerRemoved(uint32_t id)
{
    auto it = _playerTiles.find(id);
    std::pair<int, int> tile{-1, -1};
    if (it != _playerTiles.end()) tile = it->second;
    _playerTiles.erase(id);
    _playerOrient.erase(id);
    if (tile.first >= 0) restackTile(tile.first, tile.second, MIN_MOVE_DURATION);
}

void PlayerTileSystem::repositionAll()
{
    for (auto& [id, tile] : _playerTiles) {
        auto it = _playerOrient.find(id);
        int orient = (it != _playerOrient.end()) ? it->second : 1;
        sendRotateEvent(id, orientationToYaw(orient), 0.0f,
                        _tileMap.tileUp(tile.first, tile.second),
                        _tileMap.tileForward(tile.first, tile.second));
        restackTile(tile.first, tile.second, 0.0f);
    }
}

std::pair<int, int> PlayerTileSystem::getTile(uint32_t id) const
{
    auto it = _playerTiles.find(id);
    return (it != _playerTiles.end()) ? it->second : std::make_pair(-1, -1);
}

bool PlayerTileSystem::hasTile(uint32_t id) const
{
    return _playerTiles.count(id) > 0;
}

bool PlayerTileSystem::hasPlayerAt(int x, int y) const
{
    for (const auto& [id, tile] : _playerTiles)
        if (tile.first == x && tile.second == y)
            return true;
    return false;
}

float PlayerTileSystem::orientationToYaw(int o)
{
    switch (o) {
        case 1: return 0.0f;
        case 2: return 90.0f;
        case 3: return 180.0f;
        case 4: return 270.0f;
        default: return 0.0f;
    }
}

void PlayerTileSystem::restackTile(int x, int y, float duration)
{
    std::vector<uint32_t> ids;
    for (const auto& [id, tile] : _playerTiles)
        if (tile.first == x && tile.second == y) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    int count = static_cast<int>(ids.size());
    for (int i = 0; i < count; ++i)
        sendMoveEvent(ids[i], slotPos(x, y, i, count), duration);
}

void PlayerTileSystem::sendMoveEvent(uint32_t id, const graphic::Vector3f& target,
                                      float duration)
{
    _entities.handleEvent(event::Event{event::LogicEvent{
        event::EntityMoveToEvent{id, target, duration}
    }});
}

void PlayerTileSystem::sendRotateEvent(uint32_t id, float yaw, float duration,
                                        const graphic::Vector3f& up,
                                        const graphic::Vector3f& forward)
{
    _entities.handleEvent(event::Event{event::LogicEvent{
        event::EntityRotateToEvent{id, yaw, duration, up, forward}
    }});
}

graphic::Vector3f PlayerTileSystem::slotPos(int x, int y, int slot, int count) const
{
    auto center = _tileMap.standPos(x, y);
    if (count <= 1) return center;

    auto up    = _tileMap.tileUp(x, y);
    auto fwd   = _tileMap.tileForward(x, y).normalized();
    auto right = up.cross(fwd).normalized();

    float radius = PLAYER_SEPARATION / (2.0f * std::sin(static_cast<float>(M_PI) / count));
    float angle  = static_cast<float>(slot) * 2.0f * static_cast<float>(M_PI) / count;

    return center
         + right * (std::cos(angle) * radius)
         + fwd   * (std::sin(angle) * radius);
}

} // namespace zappy
