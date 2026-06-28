#include "LaunchPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

static behavior::hud::InputTextData makeField(const std::string& label, const std::string& value,
                                               std::function<void(const std::string&)> onChange)
{
    behavior::hud::InputTextData f;
    f.label     = label;
    f.value     = value;
    f.width     = 240.f;
    f.height    = 26.f;
    f.fontSize  = 12.f;
    f.onChange  = std::move(onChange);
    return f;
}

const std::vector<behavior::hud::HudElement>& LaunchProvider::getHudElements() const
{
    if (!_dirty) return _cache;
    _dirty = false;
    _cache.clear();

    _cache.push_back({behavior::hud::TextData{"Launch Game", 16.f, {120, 180, 255, 255}}});

    _cache.push_back({makeField("Port", _cfg.port, [this](const std::string& v) {
        _cfg.port = v; markDirty();
    })});
    _cache.push_back({makeField("Map Width", _cfg.width, [this](const std::string& v) {
        _cfg.width = v; markDirty();
    })});
    _cache.push_back({makeField("Map Height", _cfg.height, [this](const std::string& v) {
        _cfg.height = v; markDirty();
    })});
    _cache.push_back({makeField("Clients/team", _cfg.clients, [this](const std::string& v) {
        _cfg.clients = v; markDirty();
    })});
    _cache.push_back({makeField("Teams (space-sep)", _cfg.teams, [this](const std::string& v) {
        _cfg.teams = v; markDirty();
    })});
    _cache.push_back({makeField("Frequency", _cfg.freq, [this](const std::string& v) {
        _cfg.freq = v; markDirty();
    })});

    if (!_status.empty()) {
        _cache.push_back({behavior::hud::TextData{_status, 11.f, {255, 200, 80, 255}}});
    }

    _cache.push_back({behavior::hud::ButtonData{
        "Launch All", 14.f, 240.f, 38.f,
        {255, 255, 255, 255},
        {20, 100, 50, 220},
        {30, 160, 80, 240},
        [this]() { if (_onLaunch) _onLaunch(_cfg); }
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

void LaunchPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<LaunchProvider>();

    auto entity = EntityBuilder(hud, ID, "launch_hud")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 8.f)
        .hud().anchor(graphic::Anchor::Center)
        .hud().background(true, {8, 10, 22, 210}, {50, 60, 140, 220})
        .hud().boxSize({290.f, 420.f})
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
    if (_container) _container->setScrollable(true);
}

void LaunchPanel::setOnLaunch(std::function<void(const LaunchConfig&)> fn) {
    _onLaunch = std::move(fn);
    _provider->setOnLaunch([this](LaunchConfig cfg) {
        if (_onLaunch) _onLaunch(cfg);
    });
}
void LaunchPanel::setOnBack(std::function<void()> fn)  { _provider->setOnBack(std::move(fn)); }
void LaunchPanel::setStatus(const std::string& s)      { _provider->setStatus(s); }
void LaunchPanel::show() { if (_container) _container->setVisible(true); }
void LaunchPanel::hide() { if (_container) _container->setVisible(false); }

} // namespace zappy
