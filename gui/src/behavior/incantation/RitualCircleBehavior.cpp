#include "RitualCircleBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"
#include "event/RenderEvent.hpp"

namespace behavior {

static constexpr graphic::Color4b RITUAL_COLOR = { 255, 50, 255, 255 };

RitualCircleBehavior::RitualCircleBehavior(zappy::EntityManager& entities,
                                           graphic::EntityID selfId,
                                           graphic::MeshHandle groundMesh,
                                           graphic::Vector3f center,
                                           graphic::Vector3f surfaceNormal,
                                           float tileSpacing,
                                           int tileX, int tileY)
    : _entities(&entities), _selfId(selfId),
      _groundMesh(groundMesh), _center(center), _surfaceNormal(surfaceNormal),
      _tileSpacing(tileSpacing), _tileX(tileX), _tileY(tileY)
{}

void RitualCircleBehavior::onUpdate(graphic::Entity&, float dt)
{
    _time += dt;
}

void RitualCircleBehavior::onEvent(graphic::Entity&, const event::Event& ev)
{
    event::on(ev,
        [&](const event::IncantationEndEvent& e) {
            if (e.x == _tileX && e.y == _tileY)
                _entities->removeEntity(_selfId);
        },
        [&](const event::RenderEvent& re) {
            re.renderer.drawRitual({
                _groundMesh, _center, RITUAL_COLOR, _time, _tileSpacing * 0.9f, _surfaceNormal
            });
        }
    );
}

} // namespace behavior
