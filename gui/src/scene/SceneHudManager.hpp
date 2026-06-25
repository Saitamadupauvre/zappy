#pragma once

#include "scene/hud/chat/ChatPanel.hpp"
#include "scene/hud/leaderboard/LeaderboardPanel.hpp"
#include "scene/hud/leaderboard/WorldInfoPanel.hpp"
#include "scene/hud/player/PlayerInfoPanel.hpp"
#include "scene/hud/popup/PopupPanel.hpp"
#include "scene/hud/resource/ResourceInfoPanel.hpp"
#include "scene/hud/speed/SpeedPanel.hpp"
#include "scene/hud/clock/ClockPanel.hpp"
#include "scene/hud/leaderboard/TeamDetailPanel.hpp"
#include "scene/hud/settings/SettingsPanel.hpp"
#include "scene/hud/inventory/InventoryPanel.hpp"
#include "scene/hud/teamstats/TeamStatsPanel.hpp"
#include "core/manager/input/InputManager.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/ITextureLoader.hpp"
#include "graphic/Types.hpp"
#include "world/WorldTypes.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace zappy {

class SceneHudManager {
public:
    void setup(HudManager& hud, graphic::IRenderer& renderer,
               graphic::ITextureLoader& loader, InputManager& input);

    void setSendLine(std::function<void(std::string)> fn);
    void setSetFps(std::function<void(int)> fn);
    void setOnFovChange(std::function<void(float)> fn);
    void setOnFpsOverlay(std::function<void(bool)> fn);
    void setOnFullscreen(std::function<void(bool)> fn);
    void setOnResolution(std::function<void(int,int)> fn);
    void setOnIncantationEffect(std::function<void(bool)> fn);
    void setOnBroadcastCircle(std::function<void(bool)> fn);
    void setOnEggHatchAnim(std::function<void(bool)> fn);
    void setOnTeamColorTags(std::function<void(bool)> fn);
    void setOnSkyMode(std::function<void(int)> fn);
    void setOnGrass(std::function<void(bool)> fn);
    void setOnExitGame(std::function<void()> fn);
    void setOnSelectTeamClick(std::function<void(const std::string&)> fn) { _onSelectTeamClick = std::move(fn); }
    void setVotedTeam(const std::string& team);
    void setTimeUnit(int tu);
    void onServerUptime(unsigned long ticks);
    void tick(float dt);

    void onTeamAdded(const std::string& name, graphic::Color4b color);
    void onPlayerAdded(const PlayerState& p, graphic::Color4b color);
    void onPlayerMoved(uint32_t id, const std::string& team, int x, int y);
    void onPlayerRemoved(uint32_t id, uint32_t selectedPlayerId);
    void onPlayerLevelChanged(uint32_t id, int level);
    void onBroadcast(uint32_t id, const std::string& message);
    void onEntitySelected(uint32_t id, const std::string& team, int x, int y);
    void onPlayerInventoryChanged(uint32_t id, const Resources& inv);
    void onToggleLeaderboard();
    void onWorldInfo(std::function<WorldInfoProvider::Stats()> computeStats);
    void onToggleSettings();
    void onEscape();
    void onTeamSelected(const std::string& team, int playerCount, int maxLevel,
                        float avgLevel, const Resources& totalResources);
    void onTeamDeselected();
    void pushPopup(const std::string& title, const std::string& subtitle,
                   graphic::Color4b color = {255, 255, 255, 255});

    PopupPanel&        popup()       { return _popup; }
    LeaderboardPanel&  leaderboard() { return _leaderboard; }
    TeamDetailPanel&   teamDetail()  { return _teamDetail; }
    ChatPanel&         chat()        { return _chat; }
    PlayerInfoPanel&   playerInfo()  { return _playerInfo; }
    ResourceInfoPanel& resourceInfo(){ return _resourceInfo; }
    SpeedPanel&        speed()       { return _speed; }
    ClockPanel&        clock()       { return _clock; }
    SettingsPanel&     settings()    { return _settings; }
    InventoryPanel&    inventory()   { return _inventory; }
    TeamStatsPanel&    teamStats()   { return _teamStats; }
    WorldInfoPanel&    worldInfo()   { return _worldInfo; }

    std::string getPlayerTeam(uint32_t id) const;

private:
    PopupPanel        _popup;
    LeaderboardPanel  _leaderboard;
    TeamDetailPanel   _teamDetail;
    ChatPanel         _chat;
    PlayerInfoPanel   _playerInfo;
    ResourceInfoPanel _resourceInfo;
    SpeedPanel        _speed;
    ClockPanel        _clock;
    SettingsPanel     _settings;
    InventoryPanel    _inventory;
    TeamStatsPanel    _teamStats;
    WorldInfoPanel    _worldInfo;

    std::unordered_map<uint32_t, std::string> _playerTeams;
    std::function<void(const std::string&)>   _onSelectTeamClick;

    PlayerState _selectedPlayerState;
    bool        _hasSelectedPlayer = false;

    void openTeamDetail(const std::string& team);
    void openChat(uint32_t playerId, const std::string& team);
};

} // namespace zappy
