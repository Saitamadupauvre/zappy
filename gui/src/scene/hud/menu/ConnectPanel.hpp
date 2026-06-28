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

class ConnectProvider : public behavior::hud::IHudProvider {
public:
    void setHost(const std::string& h)        { _host = h; markDirty(); }
    void setPort(const std::string& p)        { _port = p; markDirty(); }
    void setStatus(const std::string& s)      { _status = s; markDirty(); }

    void setOnHostChange(std::function<void(const std::string&)> fn)  { _onHostChange = std::move(fn); markDirty(); }
    void setOnPortChange(std::function<void(const std::string&)> fn)  { _onPortChange = std::move(fn); markDirty(); }
    void setOnConnect(std::function<void()> fn)   { _onConnect = std::move(fn); markDirty(); }
    void setOnBack(std::function<void()> fn)      { _onBack    = std::move(fn); markDirty(); }

    const std::string& getHost() const { return _host; }
    const std::string& getPort() const { return _port; }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override;
    uint64_t getVersion() const override { return _version; }

private:
    void markDirty() { ++_version; _dirty = true; }

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    std::string _host   = "localhost";
    std::string _port   = "";
    std::string _status = "";

    std::function<void(const std::string&)> _onHostChange;
    std::function<void(const std::string&)> _onPortChange;
    std::function<void()>                   _onConnect;
    std::function<void()>                   _onBack;
};

class ConnectPanel {
public:
    static constexpr graphic::EntityID ID = 9881;

    void setup(HudManager& hud);

    void setOnConnect(std::function<void(const std::string&, uint16_t)> fn);
    void setOnBack(std::function<void()> fn);

    void setHost(const std::string& host);
    void setPort(const std::string& port);
    void setStatus(const std::string& status);

    void show();
    void hide();

private:
    std::shared_ptr<ConnectProvider>              _provider;
    std::shared_ptr<behavior::HudContainerBehavior> _container;

    std::function<void(const std::string&, uint16_t)> _onConnect;
};

} // namespace zappy
