#include "EndScreenPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

const std::vector<behavior::hud::HudElement>& EndScreenProvider::getHudElements() const
{
    if (!_dirty) return _cache;
    _dirty = false;
    _cache.clear();

    graphic::Color4b gold   = {255, 215, 0,   255};
    graphic::Color4b white  = {235, 235, 235, 255};
    graphic::Color4b dim    = {170, 170, 170, 255};
    graphic::Color4b green  = {90,  210, 90,  255};
    graphic::Color4b red    = {220, 70,  70,  255};
    graphic::Color4b sep    = {80,  80,  80,  255};

    _cache.push_back({behavior::hud::TextData{"GAME OVER", 48.f, gold}});
    _cache.push_back({behavior::hud::RectData{500.f, 2.f, {0,0,0,0}, sep}});

    _cache.push_back({behavior::hud::TextData{"Winner", 16.f, dim}});
    _cache.push_back({behavior::hud::TextData{_info.winnerTeam, 34.f, _info.teamColor}});
    _cache.push_back({behavior::hud::RectData{500.f, 2.f, {0,0,0,0}, sep}});

    auto secs = static_cast<unsigned long>(_info.elapsedSecs);
    auto m = secs / 60;
    auto s = secs % 60;
    _cache.push_back({behavior::hud::TextData{
        std::format("Duration:  {:02}:{:02}", m, s), 18.f, white}});

    if (_info.totalPlayers > 0) {
        _cache.push_back({behavior::hud::TextData{
            "Players alive:  " + std::to_string(_info.totalPlayers), 18.f, white}});
    }

    if (!_info.winnerTeam.empty()) {
        if (_info.voteCorrect)
            _cache.push_back({behavior::hud::TextData{"Your vote was correct!", 17.f, green}});
        else
            _cache.push_back({behavior::hud::TextData{"Your vote was wrong.", 17.f, red}});
    }

    _cache.push_back({behavior::hud::RectData{500.f, 2.f, {0,0,0,0}, sep}});

    {
        behavior::hud::ButtonData btn;
        btn.label        = "Look at the world";
        btn.fontSize     = 16.f;
        btn.width        = 240.f;
        btn.height       = 36.f;
        btn.bgColor      = {40, 100, 60, 240};
        btn.hoverBgColor = {60, 155, 90, 255};
        btn.onClick      = _onLookAtWorld;
        _cache.push_back({btn});
    }
    {
        behavior::hud::ButtonData btn;
        btn.label        = "Back to Menu";
        btn.fontSize     = 16.f;
        btn.width        = 240.f;
        btn.height       = 36.f;
        btn.bgColor      = {50, 60, 130, 240};
        btn.hoverBgColor = {75, 90, 200, 255};
        btn.onClick      = _onBackToMenu;
        _cache.push_back({btn});
    }

    return _cache;
}

const std::vector<behavior::hud::HudElement>& EndScreenReturnProvider::getHudElements() const
{
    if (!_dirty) return _cache;
    _dirty = false;
    _cache.clear();

    behavior::hud::ButtonData btn;
    btn.label        = "View Results";
    btn.fontSize     = 14.f;
    btn.width        = 160.f;
    btn.height       = 30.f;
    btn.bgColor      = {120, 90, 10, 220};
    btn.hoverBgColor = {200, 160, 20, 255};
    btn.textColor    = {255, 230, 80, 255};
    btn.onClick      = _onReturn;
    _cache.push_back({btn});

    return _cache;
}

void EndScreenPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<EndScreenProvider>();

    auto entity = EntityBuilder(hud, ID, "endscreen_hud")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 22.f)
        .hud().anchor(graphic::Anchor::Center)
        .hud().background(true, {5, 8, 18, 255}, {0, 0, 0, 0})
        .hud().fullscreen()
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();

    _returnProvider = std::make_shared<EndScreenReturnProvider>();

    auto returnEntity = EntityBuilder(hud, RETURN_ID, "endscreen_return_hud")
        .hud().container(_returnProvider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 4.f)
        .hud().anchor(graphic::Anchor::TopRight)
        .hud().anchorOffset({10.f, 10.f})
        .hud().background(true, {20, 15, 5, 220}, {120, 90, 10, 200})
        .hud().autoSize()
        .hud().hidden()
        .build();

    _returnContainer = returnEntity->getBehavior<behavior::HudContainerBehavior>();
}

void EndScreenPanel::show(const EndScreenProvider::Info& info)
{
    if (_provider) {
        _provider->setInfo(info);
        _provider->setOnBackToMenu(_onBackToMenu);
        _provider->setOnLookAtWorld([this]() { enterWorldView(); });
    }
    if (_returnContainer) _returnContainer->setVisible(false);
    if (_container) _container->setVisible(true);
}

void EndScreenPanel::enterWorldView()
{
    if (_onLookAtWorld) _onLookAtWorld();
    if (_container) _container->setVisible(false);

    _returnProvider->setOnReturn([this]() { returnToEndScreen(); });
    if (_returnContainer) _returnContainer->setVisible(true);
}

void EndScreenPanel::returnToEndScreen()
{
    if (_onReturnToEndScreen) _onReturnToEndScreen();
    if (_returnContainer) _returnContainer->setVisible(false);
    if (_container) _container->setVisible(true);
}

void EndScreenPanel::hide()
{
    if (_container) _container->setVisible(false);
    if (_returnContainer) _returnContainer->setVisible(false);
}

bool EndScreenPanel::isVisible() const
{
    return (_container && _container->isVisible())
        || (_returnContainer && _returnContainer->isVisible());
}

void EndScreenPanel::setOnBackToMenu(std::function<void()> fn)
{
    _onBackToMenu = std::move(fn);
    if (_provider) _provider->setOnBackToMenu(_onBackToMenu);
}

void EndScreenPanel::setOnLookAtWorld(std::function<void()> fn)
{
    _onLookAtWorld = std::move(fn);
}

void EndScreenPanel::setOnReturnToEndScreen(std::function<void()> fn)
{
    _onReturnToEndScreen = std::move(fn);
}

} // namespace zappy
