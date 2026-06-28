#pragma once

#include "Scene.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/ITextureLoader.hpp"
#include "logger/ContextLogger.hpp"
#include "scene/hud/menu/MainMenuPanel.hpp"
#include "scene/hud/menu/ConnectPanel.hpp"
#include "scene/hud/menu/LaunchPanel.hpp"
#include "core/ProcessSpawner.hpp"
#include <cstdint>
#include <functional>
#include <string>

namespace zappy {

class MenuScene : public Scene
{
public:
    MenuScene(graphic::IRenderer& renderer, graphic::ITextureLoader& textureLoader,
              const std::string& preHost = "", const std::string& prePort = "");
    ~MenuScene() override;

    void update(const World& world, float dt) override;
    void render(graphic::IRenderer& renderer) override;
    void handleEvent(const event::Event& ev) override;

    void setOnConnect(std::function<void(const std::string&, uint16_t)> fn);
    void setOnQuit(std::function<void()> fn);
    void setOnCancelConnect(std::function<void()> fn);
    void setOnProcessSpawned(std::function<void(int)> fn);

    // Called by GameEngine while in Connecting state
    void setConnectStatus(const std::string& status);

private:
    enum class MenuState { Main, Connect, Launch };

    void showMain();
    void showConnect();
    void showLaunch();
    void onLaunchAll(const LaunchConfig& cfg);

    graphic::SkyboxHandle _skyboxHandle{};
    bool                  _skyboxReady = false;
    float                 _skyboxTime  = 0.f;

    MainMenuPanel  _mainMenu;
    ConnectPanel   _connectPanel;
    LaunchPanel    _launchPanel;

    MenuState _state = MenuState::Main;

    ProcessSpawner _spawner;

    std::function<void(const std::string&, uint16_t)> _onConnect;
    std::function<void()>                             _onQuit;
    std::function<void()>                             _onCancelConnect;

    ContextLogger _log{"MenuScene"};
};

} // namespace zappy
