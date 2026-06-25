#pragma once

#include "behavior/ABehavior.hpp"
#include "graphic/Types.hpp"
#include "entity/Entity.hpp"

namespace zappy { class EntityManager; }

namespace behavior {

class WaveBroadcastBehavior : public ABehavior {
public:
    WaveBroadcastBehavior(zappy::EntityManager& em,
                          graphic::EntityID      selfId,
                          graphic::MeshHandle    groundMesh,
                          graphic::Vector3f      center,
                          graphic::Color4b       color,
                          float                  duration  = 2.5f,
                          float                  maxRadius = 3.f);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent (graphic::Entity& owner, const event::Event& ev) override;

private:
    zappy::EntityManager& _em;
    graphic::EntityID     _selfId;
    graphic::MeshHandle   _groundMesh;
    graphic::Vector3f     _center;
    graphic::Color4b      _color;
    float                 _duration;
    float                 _maxRadius;
    float                 _elapsed = 0.f;
    bool                  _done    = false;
};

} // namespace behavior
