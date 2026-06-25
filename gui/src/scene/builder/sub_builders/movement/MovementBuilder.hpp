#pragma once

#include "scene/builder/BehaviorBuilders.hpp"
#include <cstdint>

// Animation expert: server-driven translation and rotation tweening.
class MovementBuilder {
    public:
        MovementBuilder(EntityBuilder& owner, EntityPtr entity);

        EntityBuilder& move(uint32_t entityId);
        EntityBuilder& rotate(uint32_t entityId);
        EntityBuilder& animated(uint32_t entityId);

    private:
        EntityBuilder& _owner;
        EntityPtr      _entity;
};
