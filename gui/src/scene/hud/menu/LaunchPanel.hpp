#pragma once

#include "core/manager/hud/HudManager.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "graphic/Types.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace zappy {

struct LaunchConfig {
    std::string port   = "4242";
    std::string width  = "10";
    std::string height = "10";
    std::string clients = "1";
    std::string teams  = "team1 team2";
    std::string freq   = "100";
};

class LaunchProvider : public behavior::hud::IHudProvider {
public:
    LaunchConfig& config() { return _cfg; }

    void setOnFieldChange(std::function<void(LaunchConfig)> fn) { _onFieldChange = std::move(fn); }
    void setOnLaunch(std::function<void(LaunchConfig)> fn)      { _onLaunch      = std::move(fn); markDirty(); }
    void setOnBack(std::function<void()> fn)                    { _onBack        = std::move(fn); markDirty(); }
    void setStatus(const std::string& s)                        { _status = s; markDirty(); }

    void markDirty() const { ++_version; _dirty = true; }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override;
    uint64_t getVersion() const override { return _version; }

private:
    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    mutable LaunchConfig _cfg;
    std::string          _status;

    std::function<void(LaunchConfig)> _onFieldChange;
    std::function<void(LaunchConfig)> _onLaunch;
    std::function<void()>             _onBack;
};

class LaunchPanel {
public:
    static constexpr graphic::EntityID ID = 9882;

    void setup(HudManager& hud);

    void setOnLaunch(std::function<void(const LaunchConfig&)> fn);
    void setOnBack(std::function<void()> fn);
    void setStatus(const std::string& s);

    void show();
    void hide();

private:
    std::shared_ptr<LaunchProvider>               _provider;
    std::shared_ptr<behavior::HudContainerBehavior> _container;

    std::function<void(const LaunchConfig&)> _onLaunch;
};

} // namespace zappy
