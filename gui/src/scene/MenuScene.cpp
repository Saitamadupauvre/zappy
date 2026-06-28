#include "MenuScene.hpp"
#include "event/Event.hpp"
#include <sstream>

namespace zappy {

MenuScene::MenuScene(graphic::IRenderer& renderer, graphic::ITextureLoader& /*textureLoader*/,
                     const std::string& preHost, const std::string& prePort)
{
    _renderer = &renderer;

    _mainMenu.setup(_hud);
    _connectPanel.setup(_hud);
    _launchPanel.setup(_hud);

    _mainMenu.setOnLaunch([this]()  { showLaunch(); });
    _mainMenu.setOnConnect([this]() { showConnect(); });
    _mainMenu.setOnQuit([this]()    { if (_onQuit) _onQuit(); });

    _connectPanel.setOnBack([this]() {
        if (_onCancelConnect) _onCancelConnect();
        showMain();
    });
    _connectPanel.setOnConnect([this](const std::string& host, uint16_t port) {
        if (_onConnect) _onConnect(host, port);
    });

    _launchPanel.setOnBack([this]() { showMain(); });
    _launchPanel.setOnLaunch([this](const LaunchConfig& cfg) { onLaunchAll(cfg); });

    try {
        _skyboxHandle = renderer.uploadSkybox();
        _skyboxReady  = true;
    } catch (...) {
        _skyboxReady = false;
    }

    // Pre-fill from CLI args
    if (!preHost.empty()) _connectPanel.setHost(preHost);
    if (!prePort.empty()) _connectPanel.setPort(prePort);

    // Show connect panel when host or port pre-filled from CLI args
    if (!preHost.empty() || !prePort.empty())
        showConnect();
    else
        showMain();
}

MenuScene::~MenuScene()
{
    if (_skyboxReady && _renderer)
        _renderer->unloadSkybox(_skyboxHandle);
}

void MenuScene::showMain()
{
    _state = MenuState::Main;
    _mainMenu.show();
    _connectPanel.hide();
    _launchPanel.hide();
}

void MenuScene::showConnect()
{
    _state = MenuState::Connect;
    _mainMenu.hide();
    _connectPanel.show();
    _launchPanel.hide();
}

void MenuScene::showLaunch()
{
    _state = MenuState::Launch;
    _mainMenu.hide();
    _connectPanel.hide();
    _launchPanel.show();
}

void MenuScene::onLaunchAll(const LaunchConfig& cfg)
{
    std::string serverCmd = "./zappy_server";
    serverCmd += " -p " + cfg.port;
    serverCmd += " -x " + cfg.width;
    serverCmd += " -y " + cfg.height;
    serverCmd += " -c " + cfg.clients;
    serverCmd += " -n " + cfg.teams;
    serverCmd += " -f " + cfg.freq;

    _spawner.spawn(serverCmd);

    std::istringstream ss(cfg.teams);
    std::string team;
    while (ss >> team)
        _spawner.spawn("./zappy_ai -p " + cfg.port + " -n " + team);

    _launchPanel.setStatus("Launching… connecting to localhost:" + cfg.port);

    uint16_t port = 4242;
    try { port = static_cast<uint16_t>(std::stoul(cfg.port)); } catch (...) {}

    if (_onConnect) _onConnect("localhost", port);
}

void MenuScene::setOnProcessSpawned(std::function<void(int)> fn)
{
    _spawner.setOnSpawn(std::move(fn));
}

void MenuScene::update(const World& /*world*/, float dt)
{
    _skyboxTime += dt;
}

void MenuScene::render(graphic::IRenderer& renderer)
{
    _renderer = &renderer;
    renderer.setCamera(_camera.toCameraState());

    graphic::Vector2f vp = renderer.getViewportSize();

    renderer.begin3D();
    if (_skyboxReady) renderer.drawSkybox(_skyboxHandle, _skyboxTime);
    renderer.end3D();

    renderer.begin2D();
    _hud.handleEvent(event::RenderEvent{renderer, vp});
    renderer.end2D();
}

void MenuScene::handleEvent(const event::Event& ev)
{
    Scene::handleEvent(ev);
}

void MenuScene::setOnConnect(std::function<void(const std::string&, uint16_t)> fn)
{
    _onConnect = std::move(fn);
}

void MenuScene::setOnQuit(std::function<void()> fn)
{
    _onQuit = std::move(fn);
}

void MenuScene::setOnCancelConnect(std::function<void()> fn)
{
    _onCancelConnect = std::move(fn);
}

void MenuScene::setConnectStatus(const std::string& status)
{
    _connectPanel.setStatus(status);
}

} // namespace zappy
