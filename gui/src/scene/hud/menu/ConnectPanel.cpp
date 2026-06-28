#include "ConnectPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"
#include <cstdint>

namespace zappy {

const std::vector<behavior::hud::HudElement>& ConnectProvider::getHudElements() const
{
    if (!_dirty) return _cache;
    _dirty = false;
    _cache.clear();

    _cache.push_back({behavior::hud::TextData{
        "Connect to Server", 16.f, {120, 180, 255, 255}
    }});

    behavior::hud::InputTextData hostField;
    hostField.label       = "Host";
    hostField.value       = _host;
    hostField.placeholder = "localhost";
    hostField.width       = 240.f;
    hostField.height      = 30.f;
    hostField.onChange    = _onHostChange;
    _cache.push_back({hostField});

    behavior::hud::InputTextData portField;
    portField.label       = "Port";
    portField.value       = _port;
    portField.placeholder = "4242";
    portField.width       = 240.f;
    portField.height      = 30.f;
    portField.onChange    = _onPortChange;
    _cache.push_back({portField});

    if (!_status.empty()) {
        _cache.push_back({behavior::hud::TextData{
            _status, 12.f, {255, 200, 80, 255}
        }});
    }

    _cache.push_back({behavior::hud::ButtonData{
        "Connect", 14.f, 240.f, 38.f,
        {255, 255, 255, 255},
        {20, 100, 80, 220},
        {30, 150, 120, 240},
        _onConnect
    }});
    _cache.push_back({behavior::hud::ButtonData{
        "Back", 14.f, 240.f, 38.f,
        {255, 255, 255, 255},
        {60, 60, 80, 200},
        {90, 90, 120, 220},
        _onBack
    }});
    return _cache;
}

void ConnectPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<ConnectProvider>();

    _provider->setOnHostChange([this](const std::string& h) {
        _provider->setHost(h);
    });
    _provider->setOnPortChange([this](const std::string& p) {
        _provider->setPort(p);
    });
    _provider->setOnConnect([this]() {
        if (!_onConnect) return;
        const auto& port = _provider->getPort();
        uint16_t p = 4242;
        if (!port.empty()) {
            try { p = static_cast<uint16_t>(std::stoul(port)); } catch (...) {}
        }
        _onConnect(_provider->getHost(), p);
    });

    auto entity = EntityBuilder(hud, ID, "connect_hud")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 10.f)
        .hud().anchor(graphic::Anchor::Center)
        .hud().background(true, {8, 10, 22, 210}, {50, 60, 140, 220})
        .hud().boxSize({280.f, 320.f})
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void ConnectPanel::setOnConnect(std::function<void(const std::string&, uint16_t)> fn) {
    _onConnect = std::move(fn);
}
void ConnectPanel::setOnBack(std::function<void()> fn) {
    _provider->setOnBack(std::move(fn));
}
void ConnectPanel::setHost(const std::string& h)   { _provider->setHost(h); }
void ConnectPanel::setPort(const std::string& p)   { _provider->setPort(p); }
void ConnectPanel::setStatus(const std::string& s) { _provider->setStatus(s); }
void ConnectPanel::show() { if (_container) _container->setVisible(true); }
void ConnectPanel::hide() { if (_container) _container->setVisible(false); }

} // namespace zappy
