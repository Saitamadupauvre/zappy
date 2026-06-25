#include "Scene.hpp"
#include "event/Event.hpp"
#include "locator/Locator.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"
#include "behavior/hud/PlaceholderProvider.hpp"
#include <cmath>

static constexpr graphic::EntityID INFO_HUD_ENTITY_ID = 9999;
static constexpr float DEG_TO_RAD = M_PI / 180.0f;

namespace zappy {

Scene::Scene()
{
    setupDefaultInputs();
}

void Scene::setupDefaultInputs()
{
    _inputManager.bindTriggerListener(InputAction::CLICK, [this]() {
        if (!_renderer) return;
        event::MouseButtonEvent clickEvent = {
            graphic::MouseBtn::LEFT,
            true,
            _lastMousePos
        };

        if (_hudPicker.tryHandleClick(*this, clickEvent)) {
            _log.info("Click handled by HUD.");
        } else if (_pickSystem.tryHandleClick(*this, *_renderer, clickEvent)) {
            _log.info("Click handled by 3D World.");
        }
    });
}

void Scene::render(graphic::IRenderer& renderer)
{
    _renderer = &renderer;
    renderer.setCamera(_camera.toCameraState());
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
        [&](const event::MouseWheelEvent&) {
            if (_hudPicker.isWheelConsumed(*this, _lastMousePos)) return;
            _camera.handleEvent(ev);
        }
    );
    event::on(ev,
        [&](const event::MouseMoveEvent&)  { _camera.handleEvent(ev); },
        [&](const event::MouseButtonEvent&){ _camera.handleEvent(ev); }
    );

    event::on(ev, [&](const event::MouseMoveEvent& e) {
        _lastMousePos = e.position;
        if (!_hudPicker.tryHandleMouseMove(*this, e.position)) {
            if (_renderer)
                _pickSystem.tryHandleMouseMove(*this, *_renderer, e.position);
        }
    });

    _inputManager.handleEvent(ev);
    // WindowEvents are handled by InputManager/HudPicker/PickSystem; no 3D entity behavior needs them.
    if (!std::holds_alternative<event::WindowEvent>(ev))
        _entities.handleEvent(ev);
    _hud.handleEvent(ev);
}

} // namespace zappy
