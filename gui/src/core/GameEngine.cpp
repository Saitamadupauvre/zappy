#include "GameEngine.hpp"
#include "event/Event.hpp"
#include "locator/Locator.hpp"
#include "logger/ConsoleSink.hpp"
#include "logger/FileSink.hpp"
#include "logger/ContextLogger.hpp"
#include "graphics/raylib/RaylibWindow.hpp"
#include "graphics/raylib/RaylibRenderer.hpp"
#include "graphics/raylib/RaylibMeshFactory.hpp"
#include "graphics/raylib/RaylibTextureLoader.hpp"
#include "graphics/raylib/RaylibFontLoader.hpp"
#include "network/client/GuiConnection.hpp"
#include "scene/WorldScene.hpp"

#include <iostream>
#include <string>
#include <string>

namespace zappy {

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

    _scene = std::make_unique<WorldScene>(
        _graphics->getRenderer(),
        _graphics->getMeshFactory(),
        _graphics->getTextureLoader()
    );

    _world.setEventDispatcher([this](const event::WorldEvent& we) {
        _scene->handleEvent(event::Event{we});
    });

    static_cast<WorldScene*>(_scene.get())->setSendLine([this](std::string line) {
        _network->sendLine(std::move(line));
    });

    auto* ws = static_cast<WorldScene*>(_scene.get());

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
}

void GameEngine::initNetwork()
{
    const auto& config = _cliParser.getConfig();
    GuiConnectionConfig netConfig;
    netConfig.host = config.machine;
    netConfig.port = static_cast<uint16_t>(std::stoul(config.port));

    _network = std::make_unique<GuiNetworkManager>(netConfig);
    _network->connect();
}

GameEngine::GameEngine(int argc, const char **argv)
    : _executor(_world)
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
    initNetwork();

    Locator::provide(_scene.get());
    Locator::provide(&_graphics->getRenderer());
    _log.info("GameEngine fully initialized and ready.");
}


void GameEngine::run()
{
    if (!_successfullyParsed) return;

    _log.info("Entering main loop.");

    while (_graphics->isOpen()) {
        _network->update(0);

        net::Message msg;
        while (_network->tryPopCommand(msg))
            _executor.execute(msg);

        _graphics->pollAndDispatch(*_scene);

        if (!_graphics->isOpen()) break;

        _graphics->beginFrame();
        float dt = _graphics->getDeltaTime();
        _scene->update(_world, dt);
        _scene->render(_graphics->getRenderer());
        _graphics->endFrame();
    }

    _log.info("Main loop exited. Application shutting down.");
}

} // namespace zappy
