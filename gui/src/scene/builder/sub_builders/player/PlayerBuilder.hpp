#pragma once

#include "scene/builder/BehaviorBuilders.hpp"
#include "graphic/Types.hpp"
#include <vector>
#include "world/WorldTypes.hpp"
#include <array>
#include <cstdint>

namespace graphic { class IRenderer; }

// Gameplay expert: player-specific behaviors (broadcast, placement, stats).
class PlayerBuilder {
    public:
        PlayerBuilder(EntityBuilder& owner, EntityPtr entity);

        EntityBuilder& state(const zappy::PlayerState& player);
        EntityBuilder& broadcast(uint32_t playerId);
        EntityBuilder& animation(graphic::IRenderer& renderer,
                                 graphic::ModelHandle model,
                                 uint32_t playerId);
        EntityBuilder& levelModel(graphic::IRenderer& renderer,
                                  std::array<graphic::ModelHandle, 8> levelModels,
                                  uint32_t playerId,
                                  int initialLevel,
                                  graphic::Color4b teamColor,
                                  std::array<std::vector<int>, 8> skinMeshIndices);

    private:
        EntityBuilder& _owner;
        EntityPtr      _entity;
};
