#include "PlayerMovementBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"

namespace behavior {

PlayerMovementBehavior::PlayerMovementBehavior(uint32_t entityId)
    : _entityId(entityId) {}

void PlayerMovementBehavior::onUpdate(graphic::Entity& owner, float dt)
{
    if (!_moving) return;
    _elapsed += dt;
    float t = (_duration > 0.0f) ? (_elapsed / _duration) : 1.0f;
    if (t >= 1.0f) { t = 1.0f; _moving = false; }

    auto transform = owner.getBehavior<TransformBehavior>();
    if (!transform) return;

    transform->setPosition({
        _startPos.x + (_endPos.x - _startPos.x) * t,
        _startPos.y + (_endPos.y - _startPos.y) * t,
        _startPos.z + (_endPos.z - _startPos.z) * t
    });
}

void PlayerMovementBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::EntityMoveToEvent& e) {
            if (e.entityId != _entityId) return;
            applyMove(owner, e.target, e.duration);
        }
    );
}

void PlayerMovementBehavior::applyMove(graphic::Entity& owner,
                                        const graphic::Vector3f& target, float duration)
{
    auto transform = owner.getBehavior<TransformBehavior>();
    if (!transform) return;

    if (duration <= 0.0f) {
        transform->setPosition(target);
        _moving = false;
        return;
    }

    _startPos = transform->getPosition();
    _endPos   = target;
    _elapsed  = 0.0f;
    _duration = duration;
    _moving   = true;
}

} // namespace behavior
