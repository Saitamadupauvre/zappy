#include "PlayerOrientationBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"

namespace behavior {

PlayerOrientationBehavior::PlayerOrientationBehavior(uint32_t entityId)
    : _entityId(entityId) {}

void PlayerOrientationBehavior::onUpdate(graphic::Entity& owner, float dt)
{
    if (!_rotating) return;
    _elapsed += dt;
    float t = (_duration > 0.0f) ? (_elapsed / _duration) : 1.0f;
    if (t >= 1.0f) { t = 1.0f; _rotating = false; }

    auto transform = owner.getBehavior<TransformBehavior>();
    if (!transform) return;

    float yaw = _startYaw + (_targetYaw - _startYaw) * t;
    auto rot = transform->getRotation();
    transform->setRotation({rot.x, yaw, rot.z});
}

void PlayerOrientationBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::EntityRotateToEvent& e) {
            if (e.entityId != _entityId) return;
            auto transform = owner.getBehavior<TransformBehavior>();
            if (transform) _startYaw = transform->getRotation().y;

            if (e.duration <= 0.0f) {
                if (transform) transform->setRotation({0.0f, e.targetYaw, 0.0f});
                _rotating = false;
                return;
            }

            _targetYaw = e.targetYaw;
            _elapsed   = 0.0f;
            _duration  = e.duration;
            _rotating  = true;
        }
    );
}

} // namespace behavior
