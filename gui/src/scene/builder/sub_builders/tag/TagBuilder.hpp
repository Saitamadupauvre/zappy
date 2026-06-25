#pragma once

#include "scene/builder/BehaviorBuilders.hpp"
#include <cstdint>
#include "behavior/tag/TagBehavior.hpp"
#include "locator/Locator.hpp"

class TagBuilder {
    public:
        TagBuilder(EntityBuilder& owner, EntityPtr entity);

        EntityBuilder& link(uint32_t tagEntityId, float offsetY);

    private:
        EntityBuilder& _owner;
        EntityPtr      _entity;
};
