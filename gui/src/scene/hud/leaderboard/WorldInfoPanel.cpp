#include "WorldInfoPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void WorldInfoPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<WorldInfoProvider>();

    auto entity = EntityBuilder(hud, WORLD_INFO_HUD_ID, "world_info")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 6.0f)
        .hud().anchor(graphic::Anchor::TopRight)
        .hud().anchorOffset({10.0f, 175.0f})
        .hud().background(true, {10, 10, 20, 220}, {60, 160, 220, 240})
        .hud().boxSize({280.0f, 400.0f})
        .hud().title("World Info", 13.0f)
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void WorldInfoPanel::show(const WorldInfoProvider::Stats& stats)
{
    if (_provider) _provider->update(stats);
    if (_container) _container->setVisible(true);
}

void WorldInfoPanel::hide()
{
    if (_container) _container->setVisible(false);
}

void WorldInfoPanel::toggle(const WorldInfoProvider::Stats& stats)
{
    if (!_container) return;
    if (_container->isFullyVisible())
        hide();
    else
        show(stats);
}

bool WorldInfoPanel::isVisible() const
{
    return _container && _container->isVisible();
}

} // namespace zappy
