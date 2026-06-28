#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/Types.hpp"
#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace zappy {

class EndScreenProvider : public behavior::hud::IHudProvider {
public:
    struct Info {
        std::string          winnerTeam;
        graphic::Color4b     teamColor    = {255, 220, 80, 255};
        bool                 voteCorrect  = false;
        double               elapsedSecs  = 0.0;
        int                  totalPlayers = 0;
    };

    void setInfo(const Info& info) { _info = info; markDirty(); }
    void setOnBackToMenu(std::function<void()> fn)  { _onBackToMenu = std::move(fn); }
    void setOnLookAtWorld(std::function<void()> fn) { _onLookAtWorld = std::move(fn); }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override;
    uint64_t getVersion() const override { return _version; }

private:
    Info _info;
    std::function<void()> _onBackToMenu;
    std::function<void()> _onLookAtWorld;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() { ++_version; _dirty = true; }
};

// Small always-on-top button shown while looking at the world after game ends
class EndScreenReturnProvider : public behavior::hud::IHudProvider {
public:
    void setOnReturn(std::function<void()> fn) { _onReturn = std::move(fn); markDirty(); }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override;
    uint64_t getVersion() const override { return _version; }

private:
    std::function<void()> _onReturn;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() { ++_version; _dirty = true; }
};

class EndScreenPanel {
public:
    static constexpr graphic::EntityID ID        = 9878;
    static constexpr graphic::EntityID RETURN_ID = 9877;

    void setup(HudManager& hud);
    void show(const EndScreenProvider::Info& info);
    void hide();
    // Called by "Look at the world": hides main overlay, shows the return button
    void enterWorldView();
    // Called by the return button: hides return button, re-shows main overlay
    void returnToEndScreen();
    bool isVisible() const;

    void setOnBackToMenu(std::function<void()> fn);
    void setOnLookAtWorld(std::function<void()> fn);
    void setOnReturnToEndScreen(std::function<void()> fn);

private:
    std::shared_ptr<EndScreenProvider>               _provider;
    std::shared_ptr<behavior::HudContainerBehavior>  _container;

    std::shared_ptr<EndScreenReturnProvider>          _returnProvider;
    std::shared_ptr<behavior::HudContainerBehavior>   _returnContainer;

    std::function<void()> _onBackToMenu;
    std::function<void()> _onLookAtWorld;
    std::function<void()> _onReturnToEndScreen;
};

} // namespace zappy
