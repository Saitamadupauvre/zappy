#include "ChatPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void ChatPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<TeamChatProvider>(&_chatStore);

    auto entity = EntityBuilder(hud, CHAT_HUD_ID, "chat_hud")
        .hud().rect({0, 0}, {340, 460})
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 6.0f)
        .hud().background(true, {8, 8, 22, 210}, {55, 55, 110, 220})
        .hud().anchor(graphic::Anchor::TopRight)
        .hud().anchorOffset({0.0f, 270.0f})
        .hud().boxSize({340.0f, 400.0f})
        .hud().title("Team Chat", 14.0f)
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
    if (_container)
        _container->setScrollable(true);
}

void ChatPanel::open(uint32_t playerId, const std::string& team)
{
    if (_container && _container->isVisible()) {
        _container->setVisible(false);
        return;
    }
    _provider->setTeam(team);
    _provider->setViewedPlayerId(playerId);
    if (_container) {
        _container->setVisible(true);
        _container->scrollToBottom();
    }
}

void ChatPanel::close()
{
    if (_container) _container->setVisible(false);
}

void ChatPanel::onBroadcast(uint32_t id, const std::string& message)
{
    _chatStore.addMessage(id, message);
    if (_container && _container->isVisible()) {
        auto team = _chatStore.getTeamForPlayer(id);
        if (_provider && team == _provider->getTeam())
            _container->scrollToBottom();
    }
}

void ChatPanel::setPlayerTeam(uint32_t id, const std::string& team)
{
    _chatStore.setPlayerTeam(id, team);
}

void ChatPanel::removePlayer(uint32_t id)
{
    _chatStore.removePlayer(id);
}

bool ChatPanel::isVisible() const
{
    return _container && _container->isVisible();
}

} // namespace zappy
