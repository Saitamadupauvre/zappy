#pragma once

#include "scene/builder/BehaviorBuilders.hpp"
#include <cmath>
#include "graphic/Vectors.hpp"

namespace behavior { class TransformBehavior; }

// Geometry expert: position, rotation, scale and surface orientation.
class TransformBuilder {
    public:
        TransformBuilder(EntityBuilder& owner, EntityPtr entity);

        EntityBuilder& position(const graphic::Vector3f& pos);
        EntityBuilder& rotation(const graphic::Vector3f& rot);
        EntityBuilder& scale(const graphic::Vector3f& scale);
        EntityBuilder& scale(float uniform);
        EntityBuilder& orientation(const graphic::Vector3f& up, float yawDeg = 0.0f);

    private:
        std::shared_ptr<behavior::TransformBehavior> ensure();

        EntityBuilder& _owner;
        EntityPtr      _entity;
};
