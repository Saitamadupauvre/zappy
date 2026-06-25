#include "PlayerBuilder.hpp"
#include "entity/Entity.hpp"
#include "behavior/player/PlayerBehavior.hpp"
#include "behavior/player/BroadcastBehavior.hpp"
#include "behavior/player/PlayerLevelModelBehavior.hpp"
#include "behavior/animation/PlayerAnimationBehavior.hpp"

PlayerBuilder::PlayerBuilder(EntityBuilder& owner, EntityPtr entity)
    : _owner(owner), _entity(entity)
{
}

EntityBuilder& PlayerBuilder::state(const zappy::PlayerState& player)
{
    _entity->addBehavior<behavior::PlayerBehavior>(player);
    return _owner;
}

EntityBuilder& PlayerBuilder::broadcast(uint32_t playerId)
{
    _entity->addBehavior<behavior::BroadcastBehavior>(playerId);
    return _owner;
}

EntityBuilder& PlayerBuilder::animation(graphic::IRenderer& renderer,
                                        graphic::ModelHandle model,
                                        uint32_t playerId)
{
    _entity->addBehavior<behavior::PlayerAnimationBehavior>(renderer, model, playerId);
    return _owner;
}

EntityBuilder& PlayerBuilder::levelModel(graphic::IRenderer& renderer,
                                         std::array<graphic::ModelHandle, 8> levelModels,
                                         uint32_t playerId,
                                         int initialLevel,
                                         graphic::Color4b teamColor,
                                         std::array<std::vector<int>, 8> skinMeshIndices)
{
    _entity->addBehavior<behavior::PlayerLevelModelBehavior>(
        renderer, levelModels, playerId, initialLevel,
        teamColor, std::move(skinMeshIndices));
    return _owner;
}
