#include "MovementBuilder.hpp"
#include "entity/Entity.hpp"
#include "behavior/movement/MovementBehavior.hpp"
#include "behavior/movement/RotationBehavior.hpp"

MovementBuilder::MovementBuilder(EntityBuilder& owner, EntityPtr entity)
    : _owner(owner), _entity(entity)
{
}

EntityBuilder& MovementBuilder::move(uint32_t entityId)
{
    _entity->addBehavior<behavior::MovementBehavior>(entityId);
    return _owner;
}

EntityBuilder& MovementBuilder::rotate(uint32_t entityId)
{
    _entity->addBehavior<behavior::RotationBehavior>(entityId);
    return _owner;
}

EntityBuilder& MovementBuilder::animated(uint32_t entityId)
{
    _entity->addBehavior<behavior::MovementBehavior>(entityId);
    _entity->addBehavior<behavior::RotationBehavior>(entityId);
    return _owner;
}
