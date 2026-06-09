#include "MovementBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "Entity.hpp"

namespace behavior {

MovementBehavior::MovementBehavior(const graphic::Vector3f& velocity)
    : _velocity(velocity) {}

void MovementBehavior::setVelocity(const graphic::Vector3f& velocity) {
    _velocity = velocity;
}

void MovementBehavior::setTarget(const graphic::Vector3f& target) {
    _target = target;
    _finished = false;
}

void MovementBehavior::onUpdate(graphic::Entity& owner, float deltaTime) {
    if (_finished) return;

    auto transform = owner.getBehavior<TransformBehavior>();
    if (!transform) return;

    graphic::Vector3f pos = transform->getPosition();

    pos = pos + (_velocity * deltaTime);
    transform->setPosition(pos);

    if (pos.distanceTo(_target) < 0.1f) {
        transform->setPosition(_target); // Snap précis
        _finished = true;
    }
}

} // namespace behavior