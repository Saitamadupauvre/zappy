#include "MainMenuPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

const std::vector<behavior::hud::HudElement>& MainMenuProvider::getHudElements() const
{
    if (!_dirty) return _cache;
    _dirty = false;
    _cache.clear();

    _cache.push_back({behavior::hud::TextData{
        "ZAPPY", 32.f, {120, 180, 255, 255}
    }});
    _cache.push_back({behavior::hud::RectData{
        240.f, 2.f, {80, 80, 160, 180}, {80, 80, 160, 180}
    }});
    _cache.push_back({behavior::hud::ButtonData{
        "Launch Game", 15.f, 220.f, 42.f,
        {255, 255, 255, 255},
        {30,  90, 200, 220},
        {60, 130, 255, 240},
        _onLaunch
    }});
    _cache.push_back({behavior::hud::ButtonData{
        "Connect to Server", 15.f, 220.f, 42.f,
        {255, 255, 255, 255},
        {20, 100, 80, 220},
        {30, 150, 120, 240},
        _onConnect
    }});
    _cache.push_back({behavior::hud::ButtonData{
        "Quit", 15.f, 220.f, 42.f,
        {255, 255, 255, 255},
        {120, 30, 30, 220},
        {180, 50, 50, 240},
        _onQuit
    }});
    return _cache;
}

void MainMenuPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<MainMenuProvider>();

    auto entity = EntityBuilder(hud, ID, "main_menu_hud")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 12.f)
        .hud().anchor(graphic::Anchor::Center)
        .hud().background(true, {8, 10, 22, 210}, {50, 60, 140, 220})
        .hud().boxSize({260.f, 280.f})
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void MainMenuPanel::setOnLaunch(std::function<void()> fn)  { _provider->setOnLaunch(std::move(fn)); }
void MainMenuPanel::setOnConnect(std::function<void()> fn) { _provider->setOnConnect(std::move(fn)); }
void MainMenuPanel::setOnQuit(std::function<void()> fn)    { _provider->setOnQuit(std::move(fn)); }

void MainMenuPanel::show() { if (_container) _container->setVisible(true); }
void MainMenuPanel::hide() { if (_container) _container->setVisible(false); }

} // namespace zappy
