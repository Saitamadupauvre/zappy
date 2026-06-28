#include "SceneHudManager.hpp"
#include "event/LogicEvent.hpp"
#include "event/Event.hpp"

namespace zappy {

void SceneHudManager::setup(HudManager& hud, graphic::IRenderer& renderer,
                             graphic::ITextureLoader& loader, InputManager& input)
{
    _hud = &hud;
    _popup.setup(hud, renderer, loader);
    _leaderboard.setup(hud, renderer, loader);
    _teamDetail.setup(hud);
    _chat.setup(hud);
    _playerInfo.setup(hud, renderer, loader, [this]() {
        // onChatClick: open chat for the currently selected player
        // The scene must call onEntitySelected first so _playerTeams is populated
    });
    _resourceInfo.setup(hud, renderer, loader);
    _speed.setup(hud);
    _settings.setup(hud, input, renderer, loader);
    _settings.setOnLanguageChange([this](i18n::Language) {
        if (_hud)
            _hud->handleEvent(event::Event{event::LogicEvent{event::LanguageChangedEvent{}}});
    });
    _inventory.setup(hud, renderer, loader);
    _teamStats.setup(hud);
    _worldInfo.setup(hud);
    _clock.setup(hud);
    _loading.setup(hud);
    _endScreen.setup(hud);
    _loading.show();
}

void SceneHudManager::showLoading()
{
    _loading.show();
    _loading.setProgress(0.f);
    _loading.setText("Loading world\xe2\x80\xa6");
}

void SceneHudManager::onTileLoaded(int received, int total)
{
    if (total <= 0) return;
    float ratio = static_cast<float>(received) / static_cast<float>(total);
    _loading.setProgress(ratio);
    if (received >= total)
        _loading.hide();
}

void SceneHudManager::setSendLine(std::function<void(std::string)> fn)
{
    _speed.setSendLine(fn);
    _inventory.setSendLine(fn);
}

void SceneHudManager::setSetFps(std::function<void(int)> fn)
{
    _settings.setOnFpsChange(std::move(fn));
}

void SceneHudManager::setOnFovChange(std::function<void(float)> fn)   { _settings.setOnFovChange(std::move(fn)); }
void SceneHudManager::setOnFpsOverlay(std::function<void(bool)> fn)   { _settings.setOnFpsOverlay(std::move(fn)); }
void SceneHudManager::setOnFullscreen(std::function<void(bool)> fn)   { _settings.setOnFullscreen(std::move(fn)); }
void SceneHudManager::setOnResolution(std::function<void(int,int)> fn)   { _settings.setOnResolution(std::move(fn)); }
void SceneHudManager::setOnIncantationEffect(std::function<void(bool)> fn){ _settings.setOnIncantationEffect(std::move(fn)); }
void SceneHudManager::setOnBroadcastCircle(std::function<void(bool)> fn)  { _settings.setOnBroadcastCircle(std::move(fn)); }
void SceneHudManager::setOnEggHatchAnim(std::function<void(bool)> fn)     { _settings.setOnEggHatchAnim(std::move(fn)); }
void SceneHudManager::setOnTeamColorTags(std::function<void(bool)> fn)    { _settings.setOnTeamColorTags(std::move(fn)); }
void SceneHudManager::setOnSkyMode(std::function<void(int)> fn)           { _settings.setOnSkyMode(std::move(fn)); }
void SceneHudManager::setOnGrass(std::function<void(bool)> fn)            { _settings.setOnGrass(std::move(fn)); }
void SceneHudManager::setOnExitGame(std::function<void()> fn)             { _settings.setOnExitGame(std::move(fn)); }
void SceneHudManager::setOnSoundVolume(std::function<void(float)> fn)     { _settings.setOnSoundVolume(std::move(fn)); }
void SceneHudManager::setOnMusicVolume(std::function<void(float)> fn)     { _settings.setOnMusicVolume(std::move(fn)); }
void SceneHudManager::setOnBackToMenu(std::function<void()> fn)           { _settings.setOnBackToMenu(std::move(fn)); }

void SceneHudManager::setTimeUnit(int tu)
{
    _speed.setTimeUnit(tu);
    _clock.setTimeUnit(tu);
}

void SceneHudManager::onServerUptime(unsigned long ticks)
{
    _clock.setUptime(ticks);
}

void SceneHudManager::tick(float dt)
{
    _popup.tick(dt);
    _clock.tick(dt);
}

void SceneHudManager::onTeamAdded(const std::string& name, graphic::Color4b color)
{
    _teamColors[name] = color;
    _leaderboard.onTeamAdded(name, color);
    _leaderboard.setOnDetailsClick(name, [this, name]() {
        openTeamDetail(name);
    });
    _leaderboard.setOnSelectTeamClick(name, [this, name]() {
        if (_onSelectTeamClick) _onSelectTeamClick(name);
    });
    _leaderboard.setOnVoteClick(name, [this, name]() {
        std::string newVote = (_leaderboard.votedTeam() == name) ? "" : name;
        setVotedTeam(newVote);
    });
}

void SceneHudManager::onPlayerAdded(const PlayerState& p, graphic::Color4b /*color*/)
{
    _playerTeams[p.id] = p.team;
    _chat.setPlayerTeam(p.id, p.team);
    _leaderboard.onPlayerAdded(p.id, p.team, p.level);

    // refresh team detail if open for this team
    auto players = _leaderboard.getPlayersForTeam(p.team);
    // We don't have tile positions here; caller must call refreshTeamDetail separately
    // Just mark team membership so detail can be refreshed
    (void)players;
}

void SceneHudManager::onPlayerMoved(uint32_t id, const std::string& /*team*/,
                                     int x, int y)
{
    if (_hasSelectedPlayer && _selectedPlayerState.id == id) {
        _selectedPlayerState.x = x;
        _selectedPlayerState.y = y;
        _playerInfo.show(_selectedPlayerState);
    }
}

void SceneHudManager::onPlayerRemoved(uint32_t id, uint32_t selectedPlayerId)
{
    _chat.removePlayer(id);
    _leaderboard.onPlayerRemoved(id);
    _playerTeams.erase(id);

    if (selectedPlayerId == id) {
        _playerInfo.clear();
        _chat.close();
        _inventory.hide();
        _hasSelectedPlayer = false;
    }
}

void SceneHudManager::onPlayerLevelChanged(uint32_t id, int level)
{
    _leaderboard.onPlayerLevelChanged(id, level);
    if (_hasSelectedPlayer && _selectedPlayerState.id == id) {
        _selectedPlayerState.level = level;
        _playerInfo.show(_selectedPlayerState);
    }
}

void SceneHudManager::onPlayerInventoryChanged(uint32_t id, const Resources& inv)
{
    _inventory.onInventoryChanged(id, inv);
}

void SceneHudManager::onBroadcast(uint32_t id, const std::string& message)
{
    _chat.onBroadcast(id, message);
}

void SceneHudManager::clearSelectedPlayer()
{
    _hasSelectedPlayer = false;
    _playerInfo.clear();
    _chat.close();
    _inventory.hide();
}

void SceneHudManager::onEntitySelected(uint32_t id, const std::string& team,
                                        int x, int y)
{
    PlayerState p;
    p.id   = id;
    p.team = team;
    p.x    = x;
    p.y    = y;

    int level = 1;
    for (const auto& ps : _leaderboard.getPlayersForTeam(team)) {
        if (ps.id == id) { level = ps.level; break; }
    }
    p.level = level;

    _selectedPlayerState = p;
    _hasSelectedPlayer   = true;

    _playerInfo.show(p);
    _chat.close();
    _inventory.setSelectedPlayer(id);
}

void SceneHudManager::onToggleSettings()
{
    _settings.toggle();
}

void SceneHudManager::onEscape()
{
    if (_settings.isVisible()) {
        _settings.toggle();
        return;
    }
    if (_chat.isVisible()) {
        _chat.close();
        return;
    }
    if (_inventory.isVisible()) {
        _inventory.hide();
        return;
    }
    if (_worldInfo.isVisible()) {
        _worldInfo.hide();
        return;
    }
    if (_leaderboard.isVisible()) {
        _leaderboard.toggle();
        _teamDetail.close();
        return;
    }
    _settings.toggle();
}

void SceneHudManager::onTeamSelected(const std::string& team, int playerCount,
                                      int maxLevel, float avgLevel,
                                      const Resources& totalResources)
{
    _teamStats.show(team, playerCount, maxLevel, avgLevel, totalResources);
}

void SceneHudManager::onTeamDeselected()
{
    _teamStats.hide();
}

void SceneHudManager::onToggleLeaderboard()
{
    _leaderboard.toggle();
    if (!_leaderboard.isVisible()) {
        _teamDetail.close();
        _worldInfo.hide();
    }
}

void SceneHudManager::onWorldInfo(std::function<WorldInfoProvider::Stats()> computeStats)
{
    _worldInfo.toggle(computeStats());
}

void SceneHudManager::setOnEndScreenBackToMenu(std::function<void()> fn)
{
    _endScreen.setOnBackToMenu(std::move(fn));
}

void SceneHudManager::onGameEnded(const std::string& winnerTeam, double elapsedSecs,
                                   int totalPlayers, const std::string& votedTeam)
{
    graphic::Color4b teamColor = {255, 215, 0, 255};
    auto it = _teamColors.find(winnerTeam);
    if (it != _teamColors.end()) teamColor = it->second;

    EndScreenProvider::Info info;
    info.winnerTeam   = winnerTeam;
    info.teamColor    = teamColor;
    info.voteCorrect  = !votedTeam.empty() && (votedTeam == winnerTeam);
    info.elapsedSecs  = elapsedSecs;
    info.totalPlayers = totalPlayers;

    // Hide every other HUD panel, saving which were visible so we can restore them
    _hiddenForEndScreen.clear();
    if (_hud) {
        for (auto& entity : _hud->getEntities()) {
            auto id = entity->getID();
            if (id == EndScreenPanel::ID || id == EndScreenPanel::RETURN_ID) continue;
            auto container = entity->getBehavior<behavior::HudContainerBehavior>();
            if (container && container->isVisible()) {
                container->setVisible(false);
                _hiddenForEndScreen.push_back(container);
            }
        }
    }

    // "Look at world": restore other HUDs (end screen handles hiding itself + showing return btn)
    _endScreen.setOnLookAtWorld([this]() {
        for (auto& c : _hiddenForEndScreen)
            c->setVisible(true);
    });

    // "View Results": re-hide other HUDs when returning to end screen
    _endScreen.setOnReturnToEndScreen([this]() {
        _hiddenForEndScreen.clear();
        if (_hud) {
            for (auto& entity : _hud->getEntities()) {
                auto id = entity->getID();
                if (id == EndScreenPanel::ID || id == EndScreenPanel::RETURN_ID) continue;
                auto container = entity->getBehavior<behavior::HudContainerBehavior>();
                if (container && container->isVisible()) {
                    container->setVisible(false);
                    _hiddenForEndScreen.push_back(container);
                }
            }
        }
    });

    _endScreen.show(info);
}

void SceneHudManager::pushPopup(const std::string& title, const std::string& subtitle,
                                 graphic::Color4b color)
{
    _popup.push(title, subtitle, color);
}

std::string SceneHudManager::getPlayerTeam(uint32_t id) const
{
    auto it = _playerTeams.find(id);
    return (it != _playerTeams.end()) ? it->second : "";
}

void SceneHudManager::setVotedTeam(const std::string& team)
{
    _leaderboard.setVotedTeam(team);
    _playerInfo.setVotedTeam(team);
}

void SceneHudManager::openTeamDetail(const std::string& team)
{
    auto players = _leaderboard.getPlayersForTeam(team);
    std::vector<PlayerDetailEntry> entries;
    entries.reserve(players.size());
    for (const auto& ps : players)
        entries.push_back({ps.id, ps.level, 0, 0}); // positions filled by caller
    _teamDetail.open(team, std::move(entries));
}

void SceneHudManager::openChat(uint32_t playerId, const std::string& team)
{
    _chat.open(playerId, team);
}

} // namespace zappy
