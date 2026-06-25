#pragma once

#include "graphic/Types.hpp"
#include "scene/world/AnimationClock.hpp"
#include "scene/TileMap.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include <cstdint>
#include <unordered_map>
#include <utility>

namespace zappy {

class IMapLayout;

class PlayerTileSystem {
public:
    static constexpr float SPACING           = 2.0f;
    static constexpr float MIN_MOVE_DURATION = 0.5f;
    static constexpr float PLAYER_SEPARATION = 0.7f;

    PlayerTileSystem(EntityManager& entities, TileMap& tileMap, AnimationClock& clock);

    void setActiveLayout(IMapLayout* layout);

    void onPlayerAdded(uint32_t id, int x, int y, int orientation);
    void onPlayerMoved(uint32_t id, int x, int y, int orientation);
    void onPlayerRemoved(uint32_t id);
    void repositionAll();

    std::pair<int, int> getTile(uint32_t id) const;
    bool                hasTile(uint32_t id) const;
    bool                hasPlayerAt(int x, int y) const;

    static float orientationToYaw(int o);

private:
    void restackTile(int x, int y, float duration);
    void sendMoveEvent(uint32_t id, const graphic::Vector3f& target, float duration);
    void sendRotateEvent(uint32_t id, float yaw, float duration,
                         const graphic::Vector3f& up      = graphic::Vector3f::up(),
                         const graphic::Vector3f& forward = graphic::Vector3f::forward());
    graphic::Vector3f slotPos(int x, int y, int slot, int count) const;

    EntityManager& _entities;
    TileMap&       _tileMap;
    AnimationClock& _clock;
    IMapLayout*    _activeLayout = nullptr;

    std::unordered_map<uint32_t, std::pair<int, int>> _playerTiles;
    std::unordered_map<uint32_t, int>                 _playerOrient;
};

} // namespace zappy
