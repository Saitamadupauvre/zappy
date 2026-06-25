#include "TagBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/rectTransform/RectTransformBehavior.hpp"
#include "entity/Entity.hpp"
#include "scene/IScene.hpp"
#include "locator/Locator.hpp"
#include <iostream>

namespace behavior {

TagBehavior::TagBehavior(uint32_t tagEntityId, float offsetY)
    : _tagEntityId(tagEntityId), _offsetY(offsetY), _isSelected(false) {} // Initialisation importante !

void TagBehavior::onUpdate(graphic::Entity& owner, [[maybe_unused]] float deltaTime)
{
    auto playerTransform = owner.getBehavior<behavior::TransformBehavior>();
    if (!playerTransform) return;

    auto iScene = zappy::Locator::getScene();
    auto tagEntity = iScene->getHud().getEntity(_tagEntityId);
    if (!tagEntity) return;

    auto renderer = zappy::Locator::getRenderer();
    if (!renderer) return;

    auto container = tagEntity->getBehavior<behavior::HudContainerBehavior>();
    if (!container) return;

    container->setVisible(_isSelected);
    if (!_isSelected) return;

    graphic::Vector3f worldPos = playerTransform->getPosition();
    worldPos.y += _offsetY;
    
    graphic::Vector2f screenPos = renderer->worldToScreen(worldPos);

    graphic::Vector2f viewport = renderer->getViewportSize();
    if (screenPos.x < 0 || screenPos.x > viewport.x || 
        screenPos.y < 0 || screenPos.y > viewport.y) {
        container->setVisible(false);
        return;
    }

    auto rectTransform = tagEntity->getBehavior<behavior::RectTransformBehavior>();
    if (rectTransform) {
        graphic::Vector2f size = rectTransform->getSize();
        screenPos.x -= (size.x / 2.0f);
        screenPos.y -= (size.y + 10.0f); 
        rectTransform->setPosition(screenPos);
    }
}

void TagBehavior::onEvent(graphic::Entity& owner, const event::Event& ev) 
{
    event::on(ev,
        [&](const event::SelectEvent& e) {
            if (e.entityId != owner.getID()) return;
            _isSelected = e.isSelected;
        }
    );
}

} // namespace behavior