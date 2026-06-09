#pragma once

#include "parser/cli/CliParser.hpp"
#include "logger/Logger.hpp"
#include "locator/Locator.hpp"
#include "logger/ContextLogger.hpp"
#include "manager/input/InputManager.hpp"
#include "network/GuiNetworkManager.hpp"
#include "executor/CommandExecutor.hpp"
#include "graphics/GraphicsContext.hpp"
#include "world/World.hpp"
#include "scene/IScene.hpp"

#include <memory>
#include <string>

namespace zappy {

class GameEngine
{
    public:
        GameEngine(int argc, const char **argv);
        ~GameEngine() = default;

        void run();

        int getStatus() const noexcept { return _status; }

    private:
        static constexpr int _successStatus = 0;
        static constexpr int _errorStatus = 84;

        int _status = _successStatus;

        void initLoggerConfiguration();
        void setupDefaultInputs();
        void initGraphics();
        void initNetwork();

        Logger        _logger;
        ContextLogger _log{"GameEngine"};

        CliParser     _cliParser;
        bool          _successfullyParsed = false;

        InputManager  _inputManager;

        World         _world;
        CommandExecutor _executor;

        std::unique_ptr<GraphicsContext> _graphics;
        std::unique_ptr<GuiNetworkManager> _network;
        std::unique_ptr<IScene> _scene;
};

} // namespace zappy
