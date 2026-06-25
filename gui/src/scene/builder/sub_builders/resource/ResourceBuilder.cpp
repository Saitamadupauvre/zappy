#include "ResourceBuilder.hpp"
#include "entity/Entity.hpp"
#include "behavior/resource/ResourceBehavior.hpp"

ResourceBuilder::ResourceBuilder(EntityBuilder& owner, EntityPtr entity)
    : _owner(owner), _entity(entity)
{
}

EntityBuilder& ResourceBuilder::data(int tileX, int tileY, int resourceIdx)
{
    _entity->addBehavior<behavior::ResourceBehavior>(tileX, tileY, resourceIdx);
    return _owner;
}
