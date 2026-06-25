#include "WaveBroadcastBehavior.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "event/Event.hpp"
#include "event/RenderEvent.hpp"
#include "graphic/IRenderer.hpp"

namespace behavior {

WaveBroadcastBehavior::WaveBroadcastBehavior(zappy::EntityManager& em,
                                             graphic::EntityID      selfId,
                                             graphic::MeshHandle    groundMesh,
                                             graphic::Vector3f      center,
                                             graphic::Color4b       color,
                                             float                  duration,
                                             float                  maxRadius)
    : _em(em), _selfId(selfId), _groundMesh(groundMesh), _center(center),
      _color(color), _duration(duration), _maxRadius(maxRadius)
{}

void WaveBroadcastBehavior::onUpdate(graphic::Entity&, float dt)
{
    if (_done) return;
    _elapsed += dt;
    if (_elapsed >= _duration) {
        _done = true;
        _em.removeEntity(_selfId);
    }
}

void WaveBroadcastBehavior::onEvent(graphic::Entity&, const event::Event& ev)
{
    event::on(ev,
        [&](const event::RenderEvent& re) {
            if (_done) return;
            re.renderer.drawWave({
                _groundMesh, _center, _color,
                _elapsed, _duration, _maxRadius
            });
        }
    );
}

} // namespace behavior
