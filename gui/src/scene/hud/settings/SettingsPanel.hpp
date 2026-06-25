#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/SettingsProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "core/manager/input/InputManager.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/ITextureLoader.hpp"
#include "graphic/Types.hpp"
#include "logger/ContextLogger.hpp"
#include <memory>

namespace zappy {

class SettingsPanel {
public:
    static constexpr graphic::EntityID SETTINGS_HUD_ID = 9891;

    void setup(HudManager& hud, InputManager& input,
               graphic::IRenderer& renderer, graphic::ITextureLoader& loader);
    void setOnFpsChange(std::function<void(int)> fn);
    void setOnFovChange(std::function<void(float)> fn);
    void setOnFpsOverlay(std::function<void(bool)> fn);
    void setOnFullscreen(std::function<void(bool)> fn);
    void setOnResolution(std::function<void(int, int)> fn);

    void setOnIncantationEffect(std::function<void(bool)> fn);
    void setOnBroadcastCircle(std::function<void(bool)> fn);
    void setOnEggHatchAnim(std::function<void(bool)> fn);
    void setOnTeamColorTags(std::function<void(bool)> fn);
    void setOnSkyMode(std::function<void(int)> fn);
    void setOnGrass(std::function<void(bool)> fn);
    void setOnExitGame(std::function<void()> fn);

    void toggle();
    bool isVisible() const;

private:
    std::shared_ptr<behavior::hud::SettingsProvider>     _provider;
    std::shared_ptr<behavior::HudContainerBehavior>      _container;
    class VideoSection*         _videoSection       = nullptr;
    class GameDisplaySection*   _gameDisplaySection = nullptr;
    class PikminClickerSection* _pikminSection      = nullptr;
    ContextLogger _log{"SettingsPanel"};
};

} // namespace zappy
