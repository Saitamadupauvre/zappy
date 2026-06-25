#pragma once

#include "behavior/ABehavior.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "graphic/IRenderer.hpp"
#include <cstdint>

namespace behavior {

// Drawn on the ground during an incantation: rotating magic ritual circle
// projected onto the ground mesh via the ritual fragment shader.
// Created as a standalone entity by WorldScene on IncantationStartEvent,
// destroyed on IncantationEndEvent.
class RitualCircleBehavior : public ABehavior
{
public:
    RitualCircleBehavior(zappy::EntityManager& entities,
                         graphic::EntityID selfId,
                         graphic::MeshHandle groundMesh,
                         graphic::Vector3f center,
                         graphic::Vector3f surfaceNormal,
                         float tileSpacing,
                         int tileX, int tileY);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

private:
    zappy::EntityManager* _entities;
    graphic::EntityID     _selfId;
    graphic::MeshHandle   _groundMesh;
    graphic::Vector3f     _center;
    graphic::Vector3f     _surfaceNormal;
    float                 _tileSpacing;
    int                   _tileX;
    int                   _tileY;
    float                 _time = 0.f;
};

} // namespace behavior
