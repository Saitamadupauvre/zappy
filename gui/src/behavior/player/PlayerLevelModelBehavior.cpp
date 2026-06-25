#include "PlayerLevelModelBehavior.hpp"
#include "behavior/drawable/model/ModelDrawableBehavior.hpp"
#include "behavior/animation/PlayerAnimationBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"
#include <algorithm>

namespace behavior {

PlayerLevelModelBehavior::PlayerLevelModelBehavior(
    graphic::IRenderer& renderer,
    std::array<graphic::ModelHandle, MAX_LEVEL> levelModels,
    uint32_t playerId,
    int initialLevel,
    graphic::Color4b teamColor,
    std::array<std::vector<int>, MAX_LEVEL> skinMeshIndices)
    : _renderer(&renderer)
    , _levelModels(levelModels)
    , _playerId(playerId)
    , _currentLevel(initialLevel)
    , _teamColor(teamColor)
    , _skinMeshIndices(std::move(skinMeshIndices))
{}

void PlayerLevelModelBehavior::applyModel(graphic::Entity& owner, int level)
{
    int idx = std::clamp(level - 1, 0, MAX_LEVEL - 1);
    graphic::ModelHandle handle = _levelModels[idx];

    if (auto drawable = owner.getBehavior<ModelDrawableBehavior>()) {
        drawable->setModel(handle);
        drawable->setMeshTints(buildMeshTints(idx));
    }
    if (auto anim = owner.getBehavior<PlayerAnimationBehavior>())
        anim->setModel(handle);
}

std::vector<graphic::Color4b> PlayerLevelModelBehavior::buildMeshTints(int levelIdx) const
{
    const auto& skinIndices = _skinMeshIndices[levelIdx];
    if (skinIndices.empty())
        return {};

    int maxIdx = *std::max_element(skinIndices.begin(), skinIndices.end());
    // {0,0,0,0} sentinel = don't override this mesh's GLB material color.
    std::vector<graphic::Color4b> tints(maxIdx + 1, {0, 0, 0, 0});
    for (int i : skinIndices)
        tints[i] = _teamColor;
    return tints;
}

void PlayerLevelModelBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::PlayerLevelChangedEvent& e) {
            if (e.id != _playerId || e.level == _currentLevel) return;
            _currentLevel = e.level;
            applyModel(owner, e.level);
        }
    );
}

} // namespace behavior
