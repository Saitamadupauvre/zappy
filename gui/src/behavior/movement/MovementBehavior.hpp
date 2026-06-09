#pragma once

#include "behavior/ABehavior.hpp"
#include "graphic/Types.hpp"

namespace behavior {

class MovementBehavior : public ABehavior {
public:
    MovementBehavior(const graphic::Vector3f& velocity);
    ~MovementBehavior() override = default;

    void setVelocity(const graphic::Vector3f& velocity);
    void onUpdate(graphic::Entity& owner, float deltaTime) override;
    void setTarget(const graphic::Vector3f& target);

private:
    graphic::Vector3f _velocity;
    graphic::Vector3f _target;
    bool _finished = false;};

} // namespace behavior