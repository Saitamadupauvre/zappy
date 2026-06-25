#include "HudPicker.hpp"
#include "behavior/rectTransform/RectTransformBehavior.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hoverable/HoverableBehavior.hpp"
#include "locator/Locator.hpp"
#include "scene/Scene.hpp"
#include "entity/Entity.hpp"

namespace zappy {

bool HudPicker::tryHandleClick(Scene& scene, const event::MouseButtonEvent& e) {
    auto& hud      = scene.getHud();
    auto& entities = hud.getEntities();

    for (auto it = entities.rbegin(); it != entities.rend(); ++it) {
        auto& entity = *it;

        auto container = entity->getBehavior<behavior::HudContainerBehavior>();
        if (container && !container->isInteractable()) continue;

        auto transform = entity->getBehavior<behavior::RectTransformBehavior>();
        if (!transform) continue;

        if (isMouseOver(e.screenPos, transform->getPosition(), transform->getSize())) {
            hud.handleEvent(event::ClickEvent{ entity->getID() });
            return true;
        }
    }
    return false;
}

bool HudPicker::tryHandleMouseMove(Scene& scene, const graphic::Vector2f& mousePos) {
    bool foundHovered = false;
    auto& entityManager = scene.getEntityManager();

    for (auto& entity : scene.getHud().getEntities()) {
        auto rect = entity->getBehavior<behavior::RectTransformBehavior>();
        auto hover = entity->getBehavior<behavior::HoverableBehavior>();
        
        if (!rect || !hover) continue;

        bool isInside = isMouseOver(mousePos, rect->getPosition(), rect->getSize());

        if (isInside != hover->isHovered()) {
            hover->setHovered(isInside, *entity);
            
            entityManager.handleEvent(event::HoverEvent{entity->getID(), isInside });
        }
        
        if (isInside) {
            foundHovered = true;
        }
    }
    return foundHovered;
}

bool HudPicker::isMouseOver(const graphic::Vector2f& mousePos,
                            const graphic::Vector2f& pos,
                            const graphic::Vector2f& size) {
    return (mousePos.x >= pos.x && mousePos.x <= pos.x + size.x &&
            mousePos.y >= pos.y && mousePos.y <= pos.y + size.y);
}

bool HudPicker::isWheelConsumed(Scene& scene, const graphic::Vector2f& mousePos) const
{
    for (auto& entity : scene.getHud().getEntities()) {
        auto container = entity->getBehavior<behavior::HudContainerBehavior>();
        if (!container || !container->isInteractable() || !container->isScrollable()) continue;

        auto rect = entity->getBehavior<behavior::RectTransformBehavior>();
        if (!rect) continue;

        if (isMouseOver(mousePos, rect->getPosition(), rect->getSize()))
            return true;
    }
    return false;
}

} // namespace zappy