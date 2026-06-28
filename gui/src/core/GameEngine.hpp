#pragma once

#include "parser/cli/CliParser.hpp"
#include "logger/Logger.hpp"
#include "locator/Locator.hpp"
#include "logger/ContextLogger.hpp"
#include "network/GuiNetworkManager.hpp"
#include "executor/CommandExecutor.hpp"
#include "audio/IAudioManager.hpp"
#include <cstdint>
#include "graphics/GraphicsContext.hpp"
#include "world/World.hpp"
#include "scene/IScene.hpp"
#include <memory>

#include <memory>
#include <string>
#include <vector>
#ifdef __unix__
#include <sys/types.h>
#endif

namespace zappy {

class GameEngine
{
    public:
        GameEngine(int argc, const char **argv);
        ~GameEngine() { killSpawnedProcesses(); }

        void run();

        int getStatus() const noexcept { return _status; }

        IScene& getCurrentScene() { return *_scene; }

        void switchToMenu(const std::string& reason = "");
        void switchToGame();
        void startConnecting(const std::string& host, uint16_t port);
        void cancelConnecting();

    private:
        enum class AppState { Menu, Connecting, InGame };

        static constexpr int _successStatus = 0;
        static constexpr int _errorStatus = 84;

        int _status = _successStatus;

        void initLoggerConfiguration();
        void initGraphics();
        void createMenuScene();
        void wireWorldScene(IScene* scene);

        Logger        _logger;
        ContextLogger _log{"GameEngine"};

        CliParser     _cliParser;
        bool          _successfullyParsed = false;

        World         _world;
        std::unique_ptr<CommandExecutor> _executor;

        std::unique_ptr<GraphicsContext>    _graphics;
        std::unique_ptr<GuiNetworkManager>  _network;
        std::unique_ptr<IScene>             _scene;
        std::unique_ptr<audio::IAudioManager> _audioMgr;

        AppState _appState{AppState::Menu};

        float _connectRetryTimer = 0.f;
        static constexpr float RETRY_INTERVAL = 3.f;

        std::string _connectHost;
        uint16_t    _connectPort = 0;

        bool        _pendingSwitchToMenu  = false;
        std::string _pendingSwitchReason;
        bool        _insideEventDispatch  = false;
        bool        _gameEndedNormally    = false;

#ifdef __unix__
        std::vector<pid_t> _spawnedPids;
#endif
        void killSpawnedProcesses();
        void registerSpawnedPid(int pid);
};

} // namespace zappy
