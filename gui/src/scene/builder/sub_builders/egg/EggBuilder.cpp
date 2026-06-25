#include "EggBuilder.hpp"
#include "entity/Entity.hpp"
#include "behavior/egg/EggBehavior.hpp"

EggBuilder::EggBuilder(EntityBuilder& owner, EntityPtr entity)
    : _owner(owner), _entity(entity)
{
}

EntityBuilder& EggBuilder::data(uint32_t eggId)
{
    _entity->addBehavior<behavior::EggBehavior>(eggId);
    return _owner;
}
