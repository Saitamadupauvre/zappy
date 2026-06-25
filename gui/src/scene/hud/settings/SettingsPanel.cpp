#include "SettingsPanel.hpp"
#include "AudioSection.hpp"
#include "CreditsSection.hpp"
#include "GameDisplaySection.hpp"
#include "KeybindingsSection.hpp"
#include "PikminClickerSection.hpp"
#include "VideoSection.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void SettingsPanel::setup(HudManager& hud, InputManager& input,
                           graphic::IRenderer& renderer, graphic::ITextureLoader& loader)
{
    _provider = std::make_shared<behavior::hud::SettingsProvider>();
    _provider->addSection(std::make_shared<KeybindingsSection>(input));

    auto video = std::make_shared<VideoSection>();
    _videoSection = video.get();
    _provider->addSection(std::move(video));

    _provider->addSection(std::make_shared<AudioSection>());

    {
        auto gameDisplay = std::make_shared<GameDisplaySection>();
        _gameDisplaySection = gameDisplay.get();
        _provider->addSection(std::move(gameDisplay));
    }

    _provider->addSection(std::make_shared<CreditsSection>());

    {
        auto pikmin = std::make_shared<PikminClickerSection>();
        _pikminSection = pikmin.get();
        try {
            auto texData = loader.loadFromFile("assets/images/pikmin.jpg");
            pikmin->setTexture(renderer.uploadTexture(texData));
        } catch (...) {
            _log.warn("Failed to load pikmin clicker texture");
        }
        _provider->addSection(std::move(pikmin));
    }

    auto entity = EntityBuilder(hud, SETTINGS_HUD_ID, "settings_hud")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 8.f)
        .hud().anchor(graphic::Anchor::Center)
        .hud().anchorOffset({0.f, 0.f})
        .hud().background(true, {10, 12, 25, 220}, {60, 70, 140, 220})
        .hud().boxSize({320.f, 480.f})
        .hud().title("Settings", 15.f)
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
    _container->setScrollable(true);

    _provider->setOnNavigate([this]() {
        _container->scrollToTop();
    });
}

void SettingsPanel::setOnFpsChange(std::function<void(int)> fn)
{
    if (_videoSection) _videoSection->setOnFpsChange(std::move(fn));
}

void SettingsPanel::setOnFovChange(std::function<void(float)> fn)
{
    if (_videoSection) _videoSection->setOnFovChange(std::move(fn));
}

void SettingsPanel::setOnFpsOverlay(std::function<void(bool)> fn)
{
    if (_videoSection) _videoSection->setOnFpsOverlay(std::move(fn));
}

void SettingsPanel::setOnFullscreen(std::function<void(bool)> fn)
{
    if (_videoSection) _videoSection->setOnFullscreen(std::move(fn));
}

void SettingsPanel::setOnResolution(std::function<void(int, int)> fn)
{
    if (_videoSection) _videoSection->setOnResolution(std::move(fn));
}

void SettingsPanel::setOnIncantationEffect(std::function<void(bool)> fn)
{
    if (_gameDisplaySection) _gameDisplaySection->setOnIncantationEffect(std::move(fn));
}

void SettingsPanel::setOnBroadcastCircle(std::function<void(bool)> fn)
{
    if (_gameDisplaySection) _gameDisplaySection->setOnBroadcastCircle(std::move(fn));
}

void SettingsPanel::setOnEggHatchAnim(std::function<void(bool)> fn)
{
    if (_gameDisplaySection) _gameDisplaySection->setOnEggHatchAnim(std::move(fn));
}

void SettingsPanel::setOnTeamColorTags(std::function<void(bool)> fn)
{
    if (_gameDisplaySection) _gameDisplaySection->setOnTeamColorTags(std::move(fn));
}

void SettingsPanel::setOnSkyMode(std::function<void(int)> fn)
{
    if (_gameDisplaySection) _gameDisplaySection->setOnSkyMode(std::move(fn));
}

void SettingsPanel::setOnGrass(std::function<void(bool)> fn)
{
    if (_gameDisplaySection) _gameDisplaySection->setOnGrass(std::move(fn));
}

void SettingsPanel::setOnExitGame(std::function<void()> fn)
{
    if (_provider) _provider->setOnQuit(std::move(fn));
}

void SettingsPanel::toggle()
{
    if (!_container) return;
    bool open = _container->isFullyVisible();
    _container->setVisible(!open);
}

bool SettingsPanel::isVisible() const
{
    return _container && _container->isFullyVisible();
}

} // namespace zappy
