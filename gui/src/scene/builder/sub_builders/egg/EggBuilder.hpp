#pragma once

#include "scene/builder/BehaviorBuilders.hpp"
#include <cstdint>

// Egg expert: hatch/die pulsation behavior.
class EggBuilder {
    public:
        EggBuilder(EntityBuilder& owner, EntityPtr entity);

        EntityBuilder& data(uint32_t eggId);

    private:
        EntityBuilder& _owner;
        EntityPtr      _entity;
};
