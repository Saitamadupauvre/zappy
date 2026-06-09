#include "CommandExecutor.hpp"
#include <iostream>

namespace zappy {

CommandExecutor::CommandExecutor(World& world)
    : _world(world)
{
    initCommands();
}

void CommandExecutor::initCommands() {
    _commandTable[net::MessageKind::Msz] = std::make_unique<Command>("msz", 2, [this](const std::vector<std::string>& a) { handleMapSize(parseArg<int>(a[0]), parseArg<int>(a[1])); });
    _commandTable[net::MessageKind::Bct] = std::make_unique<Command>("bct", 9, [this](const std::vector<std::string>& a) { handleTileContent(parseArg<int>(a[0]), parseArg<int>(a[1]), extractResources(a, 2)); });
    _commandTable[net::MessageKind::Tna] = std::make_unique<Command>("tna", 1, [this](const std::vector<std::string>& a) { handleTeamName(parseArg<std::string>(a[0])); });

    _commandTable[net::MessageKind::Pnw] = std::make_unique<Command>("pnw", 6, [this](const std::vector<std::string>& a) { handleNewPlayer(parseArg<std::string>(a[0]), parseArg<int>(a[1]), parseArg<int>(a[2]), parseArg<int>(a[3]), parseArg<int>(a[4]), parseArg<std::string>(a[5])); });
    _commandTable[net::MessageKind::Ppo] = std::make_unique<Command>("ppo", 4, [this](const std::vector<std::string>& a) { handlePlayerPosition(parseArg<std::string>(a[0]), parseArg<int>(a[1]), parseArg<int>(a[2]), parseArg<int>(a[3])); });
    _commandTable[net::MessageKind::Plv] = std::make_unique<Command>("plv", 2, [this](const std::vector<std::string>& a) { handlePlayerLevel(parseArg<std::string>(a[0]), parseArg<int>(a[1])); });
    _commandTable[net::MessageKind::Pin] = std::make_unique<Command>("pin", 10, [this](const std::vector<std::string>& a) { handlePlayerInventory(parseArg<std::string>(a[0]), parseArg<int>(a[1]), parseArg<int>(a[2]), extractResources(a, 3)); });
    _commandTable[net::MessageKind::Pex] = std::make_unique<Command>("pex", 1, [this](const std::vector<std::string>& a) { handlePlayerExpulsion(parseArg<std::string>(a[0])); });
    _commandTable[net::MessageKind::Pbc] = std::make_unique<Command>("pbc", 2, [this](const std::vector<std::string>& a) { handlePlayerBroadcast(parseArg<std::string>(a[0]), parseArg<std::string>(a[1])); });

    _commandTable[net::MessageKind::Pic] = std::make_unique<Command>("pic", 4, [this](const std::vector<std::string>& a) { handleIncantationStart(parseArg<int>(a[0]), parseArg<int>(a[1]), parseArg<int>(a[2]), extractPlayerIds(a, 3)); });
    _commandTable[net::MessageKind::Pie] = std::make_unique<Command>("pie", 3, [this](const std::vector<std::string>& a) { handleIncantationEnd(parseArg<int>(a[0]), parseArg<int>(a[1]), parseArg<int>(a[2])); });

    _commandTable[net::MessageKind::Pfk] = std::make_unique<Command>("pfk", 1, [this](const std::vector<std::string>& a) { handleEggLaying(parseArg<std::string>(a[0])); });
    _commandTable[net::MessageKind::Pdr] = std::make_unique<Command>("pdr", 2, [this](const std::vector<std::string>& a) { handleResourceDrop(parseArg<std::string>(a[0]), parseArg<int>(a[1])); });
    _commandTable[net::MessageKind::Pgt] = std::make_unique<Command>("pgt", 2, [this](const std::vector<std::string>& a) { handleResourceCollect(parseArg<std::string>(a[0]), parseArg<int>(a[1])); });
    _commandTable[net::MessageKind::Pdi] = std::make_unique<Command>("pdi", 1, [this](const std::vector<std::string>& a) { handlePlayerDeath(parseArg<std::string>(a[0])); });

    _commandTable[net::MessageKind::Enw] = std::make_unique<Command>("enw", 4, [this](const std::vector<std::string>& a) { handleEggLaid(parseArg<std::string>(a[0]), parseArg<std::string>(a[1]), parseArg<int>(a[2]), parseArg<int>(a[3])); });
    _commandTable[net::MessageKind::Ebo] = std::make_unique<Command>("ebo", 1, [this](const std::vector<std::string>& a) { handleEggConnection(parseArg<std::string>(a[0])); });
    _commandTable[net::MessageKind::Edi] = std::make_unique<Command>("edi", 1, [this](const std::vector<std::string>& a) { handleEggDeath(parseArg<std::string>(a[0])); });

    _commandTable[net::MessageKind::Sgt] = std::make_unique<Command>("sgt", 1, [this](const std::vector<std::string>& a) { handleTimeUnitRequest(parseArg<int>(a[0])); });
    _commandTable[net::MessageKind::Sst] = std::make_unique<Command>("sst", 1, [this](const std::vector<std::string>& a) { handleTimeUnitModification(parseArg<int>(a[0])); });
    _commandTable[net::MessageKind::Seg] = std::make_unique<Command>("seg", 1, [this](const std::vector<std::string>& a) { handleEndGame(parseArg<std::string>(a[0])); });
    _commandTable[net::MessageKind::Smg] = std::make_unique<Command>("smg", 1, [this](const std::vector<std::string>& a) { handleServerMessage(parseArg<std::string>(a[0])); });
    
    _commandTable[net::MessageKind::Suc] = std::make_unique<Command>("suc", 0, [this]([[maybe_unused]] const std::vector<std::string>& a) { handleUnknownCommand(); });
    _commandTable[net::MessageKind::Sbp] = std::make_unique<Command>("sbp", 0, [this]([[maybe_unused]] const std::vector<std::string>& a) { handleCommandParameterError(); });
}

void CommandExecutor::execute(const net::Message& msg) {
    auto it = _commandTable.find(msg.kind);
    if (it != _commandTable.end()) {
        it->second->execute(msg.args);
    } else {
        std::cerr << "[WARNING] No executor mapped for command: " << msg.command << std::endl;
    }
}

Resources CommandExecutor::extractResources(const std::vector<std::string>& args, size_t offset) {
    if (offset >= args.size()) return {};
    std::vector<std::string> resArgs(args.begin() + offset, args.end());
    size_t idx = 0;
    return parseArg<Resources>(resArgs, idx);
}

std::vector<std::string> CommandExecutor::extractPlayerIds(const std::vector<std::string>& args, size_t offset) {
    if (offset >= args.size()) return {};
    return std::vector<std::string>(args.begin() + offset, args.end());
}

void CommandExecutor::handleMapSize(int x, int y)
{
    _world.resize(x, y);
}

void CommandExecutor::handleTileContent(int x, int y, const Resources& res)
{
    _world.setTile(x, y, res);
}

void CommandExecutor::handleTeamName(const std::string& teamName)
{
    _world.addTeam(teamName);
}

void CommandExecutor::handleNewPlayer(const std::string& id, int x, int y, int orientation, int level, const std::string& teamName)
{
    PlayerState p;
    p.id = static_cast<uint32_t>(std::stoul(id));
    p.x = x;
    p.y = y;
    p.orientation = orientation;
    p.level = level;
    p.team = teamName;
    _world.addPlayer(std::move(p));
}

void CommandExecutor::handlePlayerPosition(const std::string& id, int x, int y, int orientation)
{
    _world.movePlayer(static_cast<uint32_t>(std::stoul(id)), x, y, orientation);
}

void CommandExecutor::handlePlayerLevel(const std::string& id, int level)
{
    _world.setPlayerLevel(static_cast<uint32_t>(std::stoul(id)), level);
}

void CommandExecutor::handlePlayerInventory(const std::string& id, int x, int y, const Resources& inv)
{
    (void)x;
    (void)y;
    _world.setPlayerInventory(static_cast<uint32_t>(std::stoul(id)), inv);
}

void CommandExecutor::handlePlayerExpulsion(const std::string& id)
{
    (void)id;
}

void CommandExecutor::handlePlayerBroadcast(const std::string& id, const std::string& message)
{
    (void)id;
    (void)message;
}

void CommandExecutor::handleIncantationStart(int x, int y, int level, const std::vector<std::string>& playerIds)
{
    (void)x;
    (void)y;
    (void)level;
    (void)playerIds;
}

void CommandExecutor::handleIncantationEnd(int x, int y, int result)
{
    (void)x;
    (void)y;
    (void)result;
}

void CommandExecutor::handleEggLaying(const std::string& playerId)
{
    (void)playerId;
}

void CommandExecutor::handleResourceDrop(const std::string& playerId, int resourceId)
{
    (void)playerId;
    (void)resourceId;
}

void CommandExecutor::handleResourceCollect(const std::string& playerId, int resourceId)
{
    (void)playerId;
    (void)resourceId;
}

void CommandExecutor::handlePlayerDeath(const std::string& playerId)
{
    _world.removePlayer(static_cast<uint32_t>(std::stoul(playerId)));
}

void CommandExecutor::handleEggLaid(const std::string& eggId, const std::string& playerId, int x, int y)
{
    EggState e;
    e.id = static_cast<uint32_t>(std::stoul(eggId));
    e.playerId = static_cast<uint32_t>(std::stoul(playerId));
    e.x = x;
    e.y = y;
    _world.addEgg(e);
}

void CommandExecutor::handleEggConnection(const std::string& eggId)
{
    (void)eggId;
}

void CommandExecutor::handleEggDeath(const std::string& eggId)
{
    _world.removeEgg(static_cast<uint32_t>(std::stoul(eggId)));
}

void CommandExecutor::handleTimeUnitRequest(int timeUnit)
{
    (void)timeUnit;
}

void CommandExecutor::handleTimeUnitModification(int timeUnit)
{
    (void)timeUnit;
}

void CommandExecutor::handleEndGame(const std::string& teamName)
{
    (void)teamName;
}

void CommandExecutor::handleServerMessage(const std::string& message)
{
    (void)message;
}

void CommandExecutor::handleUnknownCommand() {}

void CommandExecutor::handleCommandParameterError() {}

} // namespace zappy