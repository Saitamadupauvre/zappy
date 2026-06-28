#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/TeamLeaderboardProvider.hpp"
#include "behavior/hud/LeaderboardControlProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/ITextureLoader.hpp"
#include "graphic/Types.hpp"
#include "logger/ContextLogger.hpp"
#include "world/TeamLeaderboardStore.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace zappy {

class LeaderboardPanel {
public:
    // Entry entities: 9900..9949  |  control bar: 9899
    static constexpr graphic::EntityID LEADERBOARD_BASE_ID    = 9900;
    static constexpr graphic::EntityID LEADERBOARD_CTRL_ID    = 9899;
    static constexpr int               MAX_VISIBLE             = 3;

    void setup(HudManager& hud, graphic::IRenderer& renderer,
               graphic::ITextureLoader& loader);

    void onTeamAdded(const std::string& name, graphic::Color4b color);
    void onPlayerAdded(uint32_t id, const std::string& team, int level = 1);
    void onPlayerRemoved(uint32_t id);
    void onPlayerLevelChanged(uint32_t id, int level);
    void toggle();

    std::vector<TeamLeaderboardStore::PlayerDetailStat> getPlayersForTeam(
        const std::string& name) const;

    void setOnDetailsClick(const std::string& team, std::function<void()> cb);
    void setOnSelectTeamClick(const std::string& team, std::function<void()> cb);
    void setOnVoteClick(const std::string& team, std::function<void()> cb);
    void setOnWorldInfoClick(std::function<void()> cb);
    void setVotedTeam(const std::string& team);
    bool isVisible() const { return _visible; }
    const std::string& votedTeam() const { return _votedTeam; }

private:
    void spawnEntry(HudManager& hud, const std::string& team, graphic::Color4b color);
    void spawnControls(HudManager& hud);
    void recompute();
    void scrollPrev();
    void scrollNext();

    TeamLeaderboardStore _lbStore;
    std::unordered_map<std::string, graphic::EntityID>                               _entityIds;
    std::unordered_map<std::string, std::shared_ptr<TeamLeaderboardProvider>>        _providers;
    std::unordered_map<std::string, std::shared_ptr<behavior::HudContainerBehavior>> _containers;
    std::unordered_map<std::string, graphic::Color4b>                                _teamColors;
    std::unordered_map<std::string, std::function<void()>>                           _detailCallbacks;
    std::unordered_map<std::string, std::function<void()>>                           _selectTeamCallbacks;
    std::unordered_map<std::string, std::function<void()>>                           _voteCallbacks;

    std::shared_ptr<LeaderboardControlProvider>              _ctrlProvider;
    std::shared_ptr<behavior::HudContainerBehavior>          _ctrlContainer;
    std::function<void()>                                    _onWorldInfoClick;

    graphic::TextureHandle _medalTex[4]{};
    bool                   _visible      = false;
    int                    _scrollOffset = 0;
    std::string            _votedTeam;

    HudManager*   _hud = nullptr;
    ContextLogger _log{"LeaderboardPanel"};
};

} // namespace zappy
