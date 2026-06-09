#include "GameEngine.hpp"
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
#include "scene/TestCubeScene.hpp"

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

    _graphics = std::make_unique<GraphicsContext>(
        std::move(window),
        std::move(renderer),
        std::move(meshFactory),
        std::move(textureLoader),
        std::move(fontLoader)
    );

    _scene = std::make_unique<TestCubeScene>(
        _graphics->getRenderer(),
        _graphics->getMeshFactory()
    );
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
    setupDefaultInputs();
    initGraphics();
    initNetwork();

    _log.info("GameEngine fully initialized and ready.");
}

void GameEngine::setupDefaultInputs()
{
    _log.info("Setting up engine diagnostic input listeners...");

    _inputManager.bindActionListener(InputAction::MOVE_FORWARD, [this](bool isActive) {
        if (isActive)
            _log.debug("Continuous action feedback: Camera vector moving FORWARD.");
        else
            _log.debug("Continuous action feedback: Camera vector STOPPED (Key Released).");
    });

    _inputManager.bindTriggerListener(InputAction::TOGGLE_POV, [this]() {
        _log.info("Trigger event captured! Alternating viewport presentation layout.");
    });

    _inputManager.bindTriggerListener(InputAction::ZOOM_IN, [this]() {
        _log.info("Zoom event captured: Scrolling UP (Zooming In).");
    });

    _inputManager.bindTriggerListener(InputAction::ZOOM_OUT, [this]() {
        _log.info("Zoom event captured: Scrolling DOWN (Zooming Out).");
    });
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

        _graphics->pollAndDispatch(_inputManager, *_scene);

        if (!_graphics->isOpen()) break;

        _graphics->beginFrame();
        _scene->update(_world);
        _scene->render(_graphics->getRenderer());
        _graphics->endFrame();
    }

    _log.info("Main loop exited. Application shutting down.");
}

} // namespace zappy
