#include "TagBuilder.hpp"
#include "entity/Entity.hpp"
#include "behavior/tag/TagBehavior.hpp"
#include "locator/Locator.hpp"

TagBuilder::TagBuilder(EntityBuilder& owner, EntityPtr entity)
    : _owner(owner), _entity(entity)
{
}

EntityBuilder& TagBuilder::link(uint32_t tagEntityId, float offsetY) {
    _entity->addBehavior<behavior::TagBehavior>(tagEntityId, offsetY);
    return _owner;
}
