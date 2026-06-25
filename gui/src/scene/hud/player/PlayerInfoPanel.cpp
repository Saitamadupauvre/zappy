#include "PlayerInfoPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void PlayerInfoPanel::setup(HudManager& hud, graphic::IRenderer& renderer,
                             graphic::ITextureLoader& loader,
                             std::function<void()> onChatClick)
{
    _provider = std::make_shared<PlayerInfoProvider>();

    try {
        auto texData = loader.loadFromFile("assets/images/pikmin.jpg");
        _avatarTex   = renderer.uploadTexture(texData);
        _provider->setAvatar(_avatarTex, 64.0f, 64.0f);
    } catch (...) {
        _log.warn("Failed to load player avatar texture");
    }

    _provider->setOnChatClick(std::move(onChatClick));

    auto entity = EntityBuilder(hud, PLAYER_INFO_HUD_ID, "player_info_hud")
        .hud().rect({0, 0}, {220, 160})
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 8.0f)
        .hud().background(true, {10, 10, 20, 190}, {70, 70, 120, 220})
        .hud().anchor(graphic::Anchor::TopRight)
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void PlayerInfoPanel::show(const PlayerState& p)
{
    if (_provider) _provider->setPlayer(p);
    if (_container) _container->setVisible(true);
}

void PlayerInfoPanel::clear()
{
    if (_provider) _provider->clear();
    if (_container) _container->setVisible(false);
}

void PlayerInfoPanel::setOnChatClick(std::function<void()> cb)
{
    if (_provider) _provider->setOnChatClick(std::move(cb));
}

void PlayerInfoPanel::setOnInventoryClick(std::function<void()> cb)
{
    if (_provider) _provider->setOnInventoryClick(std::move(cb));
}

void PlayerInfoPanel::setVotedTeam(const std::string& team)
{
    if (_provider) _provider->setVotedTeam(team);
}

} // namespace zappy
