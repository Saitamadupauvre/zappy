#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/PlayerInfoProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/ITextureLoader.hpp"
#include "graphic/Types.hpp"
#include "logger/ContextLogger.hpp"
#include "world/WorldTypes.hpp"
#include <functional>
#include <memory>

namespace zappy {

class PlayerInfoPanel {
public:
    // Entity ID: 9998
    static constexpr graphic::EntityID PLAYER_INFO_HUD_ID = 9998;

    void setup(HudManager& hud, graphic::IRenderer& renderer,
               graphic::ITextureLoader& loader,
               std::function<void()> onChatClick);

    void show(const PlayerState& p);
    void clear();
    void setOnChatClick(std::function<void()> cb);
    void setOnInventoryClick(std::function<void()> cb);
    void setVotedTeam(const std::string& team);

    graphic::TextureHandle getAvatarTex() const { return _avatarTex; }

private:
    std::shared_ptr<PlayerInfoProvider>             _provider;
    std::shared_ptr<behavior::HudContainerBehavior> _container;
    graphic::TextureHandle                          _avatarTex{};
    ContextLogger _log{"PlayerInfoPanel"};
};

} // namespace zappy
