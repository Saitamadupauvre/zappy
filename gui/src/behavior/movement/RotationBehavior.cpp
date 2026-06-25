#include "RotationBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"
#include <cmath>

namespace behavior {

// Spherical interpolation between two unit vectors → smooth tilt of the surface normal.
static graphic::Vector3f slerpUp(const graphic::Vector3f& a, const graphic::Vector3f& b, float t)
{
    float d = a.dot(b);
    if (d >  0.9995f) return b;                // nearly identical → snap
    if (d < -1.0f) d = -1.0f;
    if (d >  1.0f) d =  1.0f;
    float theta = std::acos(d) * t;
    graphic::Vector3f rel = (b - a * d).normalized();
    return (a * std::cos(theta) + rel * std::sin(theta)).normalized();
}

RotationBehavior::RotationBehavior(uint32_t entityId)
    : _entityId(entityId) {}

void RotationBehavior::onUpdate(graphic::Entity& owner, float dt)
{
    if (!_rotating) return;
    _elapsed += dt;
    float t = (_duration > 0.0f) ? (_elapsed / _duration) : 1.0f;
    if (t >= 1.0f) { t = 1.0f; _rotating = false; }

    auto transform = owner.getBehavior<TransformBehavior>();
    if (!transform) return;

    float yaw = _startYaw + (_targetYaw - _startYaw) * t;
    transform->setOrientation(slerpUp(_startUp, _targetUp, t),
                              slerpUp(_startFwd, _targetFwd, t), yaw);
}

void RotationBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::EntityRotateToEvent& e) {
            if (e.entityId != _entityId) return;
            auto transform = owner.getBehavior<TransformBehavior>();
            graphic::Vector3f up  = e.up.normalized();
            graphic::Vector3f fwd = e.forward.normalized();

            if (e.duration <= 0.0f) {
                if (transform) transform->setOrientation(up, fwd, e.targetYaw);
                _startUp   = up;
                _targetUp  = up;
                _startFwd  = fwd;
                _targetFwd = fwd;
                _targetYaw = e.targetYaw;
                _rotating  = false;
                return;
            }

            _startUp   = _targetUp;
            _targetUp  = up;
            _startFwd  = _targetFwd;
            _targetFwd = fwd;
            _startYaw  = _targetYaw;
            _targetYaw = e.targetYaw;
            _elapsed   = 0.0f;
            _duration  = e.duration;
            _rotating  = true;
        }
    );
}

} // namespace behavior
