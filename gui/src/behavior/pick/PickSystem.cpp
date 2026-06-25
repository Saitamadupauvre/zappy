#include "PickSystem.hpp"
#include "scene/Scene.hpp"
#include "behavior/drawable/ADrawable.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "behavior/hoverable/HoverableBehavior.hpp"
#include "behavior/selectable/SelectableBehavior.hpp"
#include "event/Event.hpp"
#include <limits>
#include <cmath>

namespace zappy {

bool PickSystem::spherePreCull(const graphic::Vector3f& rayOrigin,
                                const graphic::Vector3f& rayDir,
                                const graphic::Vector3f& center, float radius)
{
    float ocx = center.x - rayOrigin.x;
    float ocy = center.y - rayOrigin.y;
    float ocz = center.z - rayOrigin.z;
    float dot = ocx * rayDir.x + ocy * rayDir.y + ocz * rayDir.z;
    float dx  = ocx - dot * rayDir.x;
    float dy  = ocy - dot * rayDir.y;
    float dz  = ocz - dot * rayDir.z;
    return (dx * dx + dy * dy + dz * dz) <= (radius * radius);
}

graphic::Entity* PickSystem::raycast(Scene& scene, graphic::IRenderer& renderer,
                                     const graphic::Vector2f& mousePos)
{
    graphic::Vector3f rayDir      = renderer.screenToWorldRay(mousePos);
    graphic::CameraState camState = renderer.getCamera();
    graphic::Vector3f rayOrigin   = camState.position;

    auto& entities = scene.getEntityManager().getEntities();

    graphic::Entity* closestHit = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    for (auto& entity : entities) {
        if (!entity->getBehavior<behavior::SelectableBehavior>() &&
            !entity->getBehavior<behavior::HoverableBehavior>())
            continue;

        auto drawable  = entity->getBehavior<behavior::ADrawable>();
        auto transform = entity->getBehavior<behavior::TransformBehavior>();

        if (!drawable || !transform || !drawable->isVisible() || drawable->getMesh().id == 0)
            continue;

        const auto& pos   = transform->getPosition();
        const auto& scale = transform->getScale();
        float radius = std::max({scale.x, scale.y, scale.z}) * 2.0f + 0.5f;

        if (!spherePreCull(rayOrigin, rayDir, pos, radius))
            continue;

        auto collision = renderer.checkRayMeshCollision(
            rayOrigin, rayDir,
            drawable->getMesh(),
            pos, scale
        );

        if (collision.hasHit && collision.distance < minDistance) {
            minDistance = collision.distance;
            closestHit  = entity.get();
        }
    }

    return closestHit;
}

bool PickSystem::tryHandleClick(Scene& scene, graphic::IRenderer& renderer,
                                const event::MouseButtonEvent& e)
{
    graphic::Entity* hit = raycast(scene, renderer, e.screenPos);

    if (hit) {
        hit->handleEvent(event::ClickEvent{ hit->getID() });
        if (!hit->getBehavior<behavior::SelectableBehavior>())
            scene.handleEvent(event::EntitySelectedEvent{ std::numeric_limits<graphic::EntityID>::max() });
        return true;
    }

    scene.handleEvent(event::EntitySelectedEvent{ std::numeric_limits<graphic::EntityID>::max() });
    return false;
}

bool PickSystem::tryHandleMouseMove(Scene& scene, graphic::IRenderer& renderer,
                                    const graphic::Vector2f& mousePos)
{
    float dx = mousePos.x - _lastMousePos.x;
    float dy = mousePos.y - _lastMousePos.y;
    if (dx * dx + dy * dy < MOUSE_MOVE_THRESHOLD * MOUSE_MOVE_THRESHOLD)
        return false;
    _lastMousePos = mousePos;

    graphic::Entity* hit = raycast(scene, renderer, mousePos);
    auto& entityManager  = scene.getEntityManager();

    for (auto& entity : entityManager.getEntities()) {
        auto hover = entity->getBehavior<behavior::HoverableBehavior>();
        if (!hover) continue;

        bool isInside = (entity.get() == hit);

        if (isInside != hover->isHovered()) {
            hover->setHovered(isInside, *entity);
            entityManager.handleEvent(event::HoverEvent{ entity->getID(), isInside });
        }
    }

    bool result = (hit != nullptr);
    return result;
}

} // namespace zappy