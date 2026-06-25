#include "InfoHudBehavior.hpp"
#include "behavior/resource/ResourceBehavior.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "scene/IScene.hpp"
#include "locator/Locator.hpp"
#include "event/AllEvent.hpp"

namespace behavior {

InfoHudBehavior::InfoHudBehavior(std::shared_ptr<ResourceInfoProvider> provider)
    : _provider(std::move(provider)) {}

void InfoHudBehavior::onUpdate([[maybe_unused]] graphic::Entity& owner, [[maybe_unused]] float deltaTime)
{
}

void InfoHudBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::EntitySelectedEvent& e) {
            auto scene = zappy::Locator::getScene();
            auto& em = scene->getEntityManager();
            auto entity = em.getEntity(e.entityId);

            auto hud = owner.getBehavior<behavior::HudContainerBehavior>();

            if (entity) {
                auto resData = entity->getBehavior<behavior::ResourceBehavior>();
                if (resData) {
                    _provider->updateData(resData->getX(), resData->getY(), resData->getType(), resData->getCount());
                    if (hud) hud->setVisible(true);
                    return;
                }
            }
            _provider->clear();
            if (hud) hud->setVisible(false);
        }
    );
}

} // namespace behavior