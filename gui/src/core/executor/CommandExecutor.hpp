#pragma once

#include "network/GuiProtocol.hpp"
#include "parser/command/Command.hpp"
#include "parser/Resources/Resources.hpp"
#include "world/World.hpp"
#include <unordered_map>
#include <memory>

namespace zappy {

class CommandExecutor
{
    public:
        explicit CommandExecutor(World& world);
        ~CommandExecutor() = default;

        void execute(const net::Message& msg);

    private:
        World& _world;
        std::unordered_map<net::MessageKind, std::unique_ptr<ICommand>> _commandTable;

        void initCommands();
        Resources extractResources(const std::vector<std::string>& args, size_t offset);
        std::vector<std::string> extractPlayerIds(const std::vector<std::string>& args, size_t offset);

        void handleMapSize(int x, int y);
        void handleTileContent(int x, int y, const Resources& res);
        void handleTeamName(const std::string& teamName);
        void handleNewPlayer(const std::string& id, int x, int y, int orientation, int level, const std::string& teamName);
        void handlePlayerPosition(const std::string& id, int x, int y, int orientation);
        void handlePlayerLevel(const std::string& id, int level);
        void handlePlayerInventory(const std::string& id, int x, int y, const Resources& inv);
        void handlePlayerExpulsion(const std::string& id);
        void handlePlayerBroadcast(const std::string& id, const std::string& message);
        void handleIncantationStart(int x, int y, int level, const std::vector<std::string>& playerIds);
        void handleIncantationEnd(int x, int y, int result);
        void handleEggLaying(const std::string& playerId);
        void handleResourceDrop(const std::string& playerId, int resourceId);
        void handleResourceCollect(const std::string& playerId, int resourceId);
        void handlePlayerDeath(const std::string& playerId);
        void handleEggLaid(const std::string& eggId, const std::string& playerId, int x, int y);
        void handleEggConnection(const std::string& eggId);
        void handleEggDeath(const std::string& eggId);
        void handleTimeUnitRequest(int timeUnit);
        void handleTimeUnitModification(int timeUnit);
        void handleEndGame(const std::string& teamName);
        void handleServerMessage(const std::string& message);
        void handleUnknownCommand();
        void handleCommandParameterError();
};

} // namespace zappy