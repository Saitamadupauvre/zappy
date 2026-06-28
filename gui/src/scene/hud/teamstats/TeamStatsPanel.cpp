#include "TeamStatsPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void TeamStatsPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<TeamStatsProvider>();

    auto entity = EntityBuilder(hud, TEAM_STATS_HUD_ID, "team_stats")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 6.0f)
        .hud().anchor(graphic::Anchor::TopRight)
        .hud().anchorOffset({10.0f, 175.0f})
        .hud().background(true, {10, 10, 20, 200}, {120, 60, 200, 220})
        .hud().boxSize({260.0f, 400.0f})
        .hud().title("Team Stats", 13.0f)
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void TeamStatsPanel::show(const std::string& team, int playerCount, int maxLevel,
                           float avgLevel, const Resources& totalResources)
{
    if (_provider)
        _provider->update(team, playerCount, maxLevel, avgLevel, totalResources);
    if (_container)
        _container->setVisible(true);
}

void TeamStatsPanel::hide()
{
    if (_provider) _provider->clear();
    if (_container) _container->setVisible(false);
}

bool TeamStatsPanel::isVisible() const
{
    return _container && _container->isVisible();
}

} // namespace zappy
