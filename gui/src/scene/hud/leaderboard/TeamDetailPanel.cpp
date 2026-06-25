#include "TeamDetailPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"
#include <functional>

namespace zappy {

void TeamDetailPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<TeamDetailProvider>();

    auto entity = EntityBuilder(hud, TEAM_DETAIL_HUD_ID, "team_detail_hud")
        .hud().rect({0, 0}, {340, 460})
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 6.0f)
        .hud().background(true, {8, 8, 22, 210}, {55, 55, 110, 220})
        .hud().anchor(graphic::Anchor::TopLeft)
        .hud().anchorOffset({440.0f, 50.0f})
        .hud().boxSize({340.0f, 460.0f})
        .hud().title("Team Details", 14.0f)
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
    if (_container)
        _container->setScrollable(true);
}

void TeamDetailPanel::open(const std::string& team, std::vector<PlayerDetailEntry> entries)
{
    _log.debug("TeamDetailPanel::open team=", team,
               " container=", (_container ? "ok" : "NULL"));

    if (!_container || !_provider) return;

    bool sameTeam = _provider->getTeam() == team;
    bool isOpen   = _container->isVisible();

    if (sameTeam && _container->isFullyVisible()) {
        _container->setVisible(false);
        return;
    }

    _provider->setTeam(team);
    _provider->setPlayers(std::move(entries));

    if (!isOpen)
        _container->setVisible(true);
    _container->scrollToBottom();
}

void TeamDetailPanel::refreshIfOpen(const std::string& team,
                                     std::vector<PlayerDetailEntry> entries)
{
    if (!_container || !_provider) return;
    if (!_container->isVisible()) return;
    if (_provider->getTeam() != team) return;

    _provider->setPlayers(std::move(entries));
}

void TeamDetailPanel::close()
{
    if (_container) _container->setVisible(false);
}

void TeamDetailPanel::setOnFollowClick(std::function<void(uint32_t)> cb)
{
    if (_provider) _provider->setOnFollowClick(std::move(cb));
}

bool TeamDetailPanel::isFullyVisible() const
{
    return _container && _container->isFullyVisible();
}

const std::string& TeamDetailPanel::getCurrentTeam() const
{
    static const std::string empty;
    return _provider ? _provider->getTeam() : empty;
}

} // namespace zappy
