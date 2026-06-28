#pragma once

#include "core/manager/hud/HudManager.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "graphic/Types.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace zappy {

class MainMenuProvider : public behavior::hud::IHudProvider {
public:
    void setOnLaunch(std::function<void()> fn)  { _onLaunch  = std::move(fn); markDirty(); }
    void setOnConnect(std::function<void()> fn) { _onConnect = std::move(fn); markDirty(); }
    void setOnQuit(std::function<void()> fn)    { _onQuit    = std::move(fn); markDirty(); }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override;
    uint64_t getVersion() const override { return _version; }

private:
    void markDirty() { ++_version; _dirty = true; }

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    std::function<void()> _onLaunch;
    std::function<void()> _onConnect;
    std::function<void()> _onQuit;
};

class MainMenuPanel {
public:
    static constexpr graphic::EntityID ID = 9880;

    void setup(HudManager& hud);

    void setOnLaunch(std::function<void()> fn);
    void setOnConnect(std::function<void()> fn);
    void setOnQuit(std::function<void()> fn);

    void show();
    void hide();

private:
    std::shared_ptr<MainMenuProvider>             _provider;
    std::shared_ptr<behavior::HudContainerBehavior> _container;
};

} // namespace zappy
