#include "GameEngine.hpp"
#include "executor/CommandExecutor.hpp"
#include "event/Event.hpp"
#include "locator/Locator.hpp"
#include "logger/ConsoleSink.hpp"
#include "logger/FileSink.hpp"
#include "logger/ContextLogger.hpp"
#include "graphics/raylib/RaylibWindow.hpp"
#include "graphics/raylib/RaylibRenderer.hpp"
#include "graphics/raylib/RaylibMeshFactory.hpp"
#include "graphics/raylib/RaylibTextureLoader.hpp"
#include "graphics/raylib/RaylibAudioManager.hpp"
#include "graphics/raylib/RaylibFontLoader.hpp"
#include "network/client/GuiConnection.hpp"
#include "scene/WorldScene.hpp"
#include "scene/MenuScene.hpp"

#include <iostream>
#include <string>
#ifdef __unix__
#include <signal.h>
#include <sys/wait.h>
#endif

namespace zappy {

void GameEngine::killSpawnedProcesses()
{
#ifdef __unix__
    for (pid_t pid : _spawnedPids) {
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, WNOHANG);
    }
    _spawnedPids.clear();
#endif
}

void GameEngine::registerSpawnedPid(int pid)
{
#ifdef __unix__
    _spawnedPids.push_back(static_cast<pid_t>(pid));
#else
    (void)pid;
#endif
}

void GameEngine::initLoggerConfiguration()
{
    const auto& config = _cliParser.getConfig();

    if (config.consoleLog.level != LogLevel::NONE) {
        auto consoleSink = std::make_shared<ConsoleSink>();
        _logger.addSink(consoleSink, config.consoleLog.level);
    }

    if (!config.fileLog.filePath.empty()) {
        auto fileSink = std::make_shared<FileSink>(config.fileLog.filePath);
        _logger.addSink(fileSink, config.fileLog.level);
    }
}

void GameEngine::initGraphics()
{
    auto window         = std::make_unique<graphic::raylib::RaylibWindow>();
    auto renderer       = std::make_unique<graphic::raylib::RaylibRenderer>();
    auto meshFactory    = std::make_unique<graphic::raylib::RaylibMeshFactory>();
    auto textureLoader  = std::make_unique<graphic::raylib::RaylibTextureLoader>();
    auto fontLoader     = std::make_unique<graphic::raylib::RaylibFontLoader>();

    window->create(1280, 720, "Zappy");
    renderer->init();

    _graphics = std::make_unique<GraphicsContext>(
        std::move(window),
        std::move(renderer),
        std::move(meshFactory),
        std::move(textureLoader),
        std::move(fontLoader)
    );

    {
        auto fontData = _graphics->getFontLoader().loadFromFile("assets/font/NotoSans-VariableFont_wdth,wght.ttf", 64);
        auto handle   = _graphics->getRenderer().uploadFont(fontData);
        Locator::provideDefaultFont(handle);
    }
    {
        auto fontData = _graphics->getFontLoader().loadFromFile("assets/font/unifont.otf", 32, true);
        auto handle   = _graphics->getRenderer().uploadFont(fontData);
        Locator::provideCjkFont(handle);
    }
}

void GameEngine::createMenuScene()
{
    if (!_audioMgr) {
        _audioMgr = std::make_unique<graphic::raylib::RaylibAudioManager>();
        _audioMgr->init();
    }

    const auto& config = _cliParser.getConfig();
    auto menuScene = std::make_unique<MenuScene>(
        _graphics->getRenderer(),
        _graphics->getTextureLoader(),
        config.machine,
        config.port
    );

    menuScene->setOnConnect([this](const std::string& host, uint16_t port) {
        startConnecting(host, port);
    });
    menuScene->setOnQuit([this]() {
        _graphics->close();
    });
    menuScene->setOnCancelConnect([this]() {
        cancelConnecting();
    });
    menuScene->setOnProcessSpawned([this](int pid) {
        registerSpawnedPid(pid);
    });

    _scene = std::move(menuScene);
    _appState = AppState::Menu;
    Locator::provide(_scene.get());
}

void GameEngine::wireWorldScene(IScene* scene)
{
    auto* ws = static_cast<WorldScene*>(scene);

    _gameEndedNormally = false;
    _world.setEventDispatcher([this](const event::WorldEvent& we) {
        if (std::holds_alternative<event::GameEndedEvent>(we))
            _gameEndedNormally = true;
        _scene->handleEvent(event::Event{we});
    });

    ws->setSendLine([this](std::string line) {
        if (_network) _network->sendLine(std::move(line));
    });
    ws->setSetFps([this](int fps) {
        _graphics->setTargetFps(fps);
    });
    ws->setOnFovChange(nullptr);
    ws->setOnFpsOverlay(nullptr);
    ws->setOnFullscreen([this](bool v) {
        _graphics->setFullscreen(v);
    });
    ws->setOnResolution([this](int w, int h) {
        _graphics->setResolution(w, h);
    });
    ws->setOnExitGame([this]() {
        _graphics->close();
    });
    ws->setOnBackToMenu([this]() {
        switchToMenu("User requested menu");
    });
}

GameEngine::GameEngine(int argc, const char **argv)
{
    _logger.setMinLevel(LogLevel::TRACE);
    Locator::provide(&_logger);
    try {
        _successfullyParsed = _cliParser.parseArguments(argc, argv);
    } catch (const CliParserException &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        _successfullyParsed = false;
    }

    if (!_successfullyParsed) {
        _status = _errorStatus;
        return;
    }

    initLoggerConfiguration();
    initGraphics();
    _executor = std::make_unique<CommandExecutor>(_world);

    const auto& config = _cliParser.getConfig();

    createMenuScene();

    // If both host and port provided, go straight to connecting
    if (!config.port.empty()) {
        uint16_t port = static_cast<uint16_t>(std::stoul(config.port));
        startConnecting(config.machine, port);
    }

    Locator::provide(&_graphics->getRenderer());
    _log.info("GameEngine fully initialized and ready.");
}

void GameEngine::startConnecting(const std::string& host, uint16_t port)
{
    _log.info("Starting connection to ", host, ":", port);
    _connectHost = host;
    _connectPort = port;
    _connectRetryTimer = 0.f;
    _appState = AppState::Connecting;

    _network.reset();
    GuiConnectionConfig netConfig;
    netConfig.host = host;
    netConfig.port = port;
    _network = std::make_unique<GuiNetworkManager>(netConfig);
    _network->connect();

    if (auto* ms = dynamic_cast<MenuScene*>(_scene.get()))
        ms->setConnectStatus("Connecting…");
}

void GameEngine::cancelConnecting()
{
    _log.info("Connection cancelled by user.");
    _network.reset();
    _appState = AppState::Menu;
    if (auto* ms = dynamic_cast<MenuScene*>(_scene.get()))
        ms->setConnectStatus("");
}

void GameEngine::switchToMenu(const std::string& reason)
{
    // Defer if called from within an event callback to avoid destroying the
    // scene while its call stack is still active (use-after-free crash).
    if (_insideEventDispatch) {
        _pendingSwitchToMenu = true;
        _pendingSwitchReason = reason;
        return;
    }
    _log.info("Switching to menu: ", reason);
    _gameEndedNormally = false;

    _network.reset();
    _world = World{};
    _executor = std::make_unique<CommandExecutor>(_world);

    createMenuScene();
}

void GameEngine::switchToGame()
{
    _log.info("Network ready — switching to game scene.");

    auto ws = std::make_unique<WorldScene>(
        _graphics->getRenderer(),
        _graphics->getMeshFactory(),
        _graphics->getTextureLoader(),
        *_audioMgr
    );
    wireWorldScene(ws.get());
    _scene = std::move(ws);
    _appState = AppState::InGame;
    Locator::provide(_scene.get());

    // Replay world state that arrived before the scene existed
    for (const auto& team : _world.getTeams())
        _scene->handleEvent(event::Event{event::WorldEvent{event::TeamAddedEvent{team}}});
    if (_world.getWidth() > 0 && _world.getHeight() > 0)
        _scene->handleEvent(event::Event{event::WorldEvent{event::WorldResizedEvent{_world.getWidth(), _world.getHeight()}}});
    for (const auto& [id, player] : _world.getPlayers())
        _scene->handleEvent(event::Event{event::WorldEvent{event::PlayerAddedEvent{player}}});
    _scene->handleEvent(event::Event{event::WorldEvent{event::TimeUnitChangedEvent{_world.getTimeUnit()}}});
}

void GameEngine::run()
{
    if (!_successfullyParsed) return;

    _log.info("Entering main loop.");

    while (_graphics->isOpen()) {
        float dt = _graphics->getDeltaTime();

        if (_network) {
            _network->update(0);

            net::Message msg;
            while (_network->tryPopCommand(msg))
                _executor->execute(msg);
        }

        if (_appState == AppState::Connecting) {
            if (_network && _network->isReady()) {
                switchToGame();
            } else if (!_network || !_network->isConnected()) {
                _connectRetryTimer += dt;
                if (_connectRetryTimer >= RETRY_INTERVAL) {
                    _connectRetryTimer = 0.f;
                    _log.info("Retrying connection…");
                    startConnecting(_connectHost, _connectPort);
                } else {
                    int secs = static_cast<int>(RETRY_INTERVAL - _connectRetryTimer) + 1;
                    if (auto* ms = dynamic_cast<MenuScene*>(_scene.get())) {
                        ms->setConnectStatus("Could not connect. Retrying in "
                                             + std::to_string(secs) + "s…");
                    }
                }
            }
        } else if (_appState == AppState::InGame && _network && !_network->isConnected()) {
            if (_gameEndedNormally) {
                // Stay on the end screen; user clicks "Back to Menu" to leave.
            } else {
                _log.info("Server disconnected — returning to menu.");
                switchToMenu("Server disconnected");
            }
        }

        _insideEventDispatch = true;
        _graphics->pollAndDispatch(*_scene);
        _insideEventDispatch = false;

        if (_pendingSwitchToMenu) {
            _pendingSwitchToMenu = false;
            std::string reason = std::move(_pendingSwitchReason);
            switchToMenu(reason);
        }

        if (!_graphics->isOpen()) break;

        _graphics->beginFrame();
        _scene->update(_world, dt);
        _scene->render(_graphics->getRenderer());
        _graphics->endFrame();
    }

    _log.info("Main loop exited. Application shutting down.");
}

} // namespace zappy
