#include "Scene.hpp"
#include "event/Event.hpp"
#include <cmath>

static constexpr float DEG2RAD = M_PI / 180.0f;

namespace zappy {

graphic::Vector3f Scene::OrbitCamera::position() const
{
    float pitchRad = pitch * DEG2RAD;
    float yawRad   = yaw   * DEG2RAD;
    return {
        target.x + distance * std::cos(pitchRad) * std::sin(yawRad),
        target.y + distance * std::sin(pitchRad),
        target.z + distance * std::cos(pitchRad) * std::cos(yawRad)
    };
}

void Scene::render(graphic::IRenderer& renderer)
{
    renderer.setCamera({
        _camera.position(),
        _camera.target,
        graphic::Vector3f::up(),
        _camera.fov
    });
    graphic::Vector2f vp = renderer.getViewportSize();
    renderer.begin3D();
    _entities.handleEvent(event::RenderEvent{renderer, vp});
    renderer.end3D();
    renderer.begin2D();
    _hud.handleEvent(event::RenderEvent{renderer, vp});
    renderer.end2D();
}

void Scene::handleEvent(const event::Event& ev)
{
    event::on(ev,
        [&](const event::MouseWheelEvent& e) {
            _camera.distance -= e.delta * 0.8f;
            if (_camera.distance < 1.0f)   _camera.distance = 1.0f;
            if (_camera.distance > 100.0f) _camera.distance = 100.0f;
        },
        [&](const event::MouseMoveEvent& e) {
            if (!_rightButtonHeld) return;
            _camera.yaw   -= e.delta.x * 0.3f;
            _camera.pitch += e.delta.y * 0.3f;
            if (_camera.pitch >  89.0f) _camera.pitch =  89.0f;
            if (_camera.pitch < -89.0f) _camera.pitch = -89.0f;
        },
        [&](const event::MouseButtonEvent& e) {
            if (e.button == graphic::MouseBtn::RIGHT)
                _rightButtonHeld = e.pressed;
        }
    );
    _entities.handleEvent(ev);
    _hud.handleEvent(ev);
}

} // namespace zappy
