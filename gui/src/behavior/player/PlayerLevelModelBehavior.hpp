#pragma once

#include "behavior/ABehavior.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/Types.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace behavior {

// Swaps the player's 3D model when their level changes (lv1–lv8).
// Owns nothing — models are loaded and owned by PlayerEntityFactory.
class PlayerLevelModelBehavior : public ABehavior {
public:
    static constexpr int MAX_LEVEL = 8;

    // skinMeshMasks[i] = bitmask of mesh indices that are skin for level (i+1).
    PlayerLevelModelBehavior(graphic::IRenderer& renderer,
                             std::array<graphic::ModelHandle, MAX_LEVEL> levelModels,
                             uint32_t playerId,
                             int initialLevel,
                             graphic::Color4b teamColor,
                             std::array<std::vector<int>, MAX_LEVEL> skinMeshIndices);

    void onUpdate(graphic::Entity&, float) override {}
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

private:
    void applyModel(graphic::Entity& owner, int level);
    std::vector<graphic::Color4b> buildMeshTints(int levelIdx) const;

    graphic::IRenderer*                               _renderer;
    std::array<graphic::ModelHandle, MAX_LEVEL>       _levelModels;
    uint32_t                                          _playerId;
    int                                               _currentLevel;
    graphic::Color4b                                  _teamColor;
    std::array<std::vector<int>, MAX_LEVEL>           _skinMeshIndices;
};

} // namespace behavior
