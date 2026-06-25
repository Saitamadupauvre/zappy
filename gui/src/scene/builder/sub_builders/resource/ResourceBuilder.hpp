#pragma once

#include "scene/builder/BehaviorBuilders.hpp"

// Resource expert: ties a drawable to its tile slot.
class ResourceBuilder {
    public:
        ResourceBuilder(EntityBuilder& owner, EntityPtr entity);

        EntityBuilder& data(int tileX, int tileY, int resourceIdx);

    private:
        EntityBuilder& _owner;
        EntityPtr      _entity;
};
