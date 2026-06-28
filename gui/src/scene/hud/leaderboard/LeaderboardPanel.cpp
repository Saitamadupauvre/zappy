#include "LeaderboardPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

static constexpr float ROW_H   = 148.0f;
static constexpr float TOP     = 10.0f;
static constexpr float LEFT    = 10.0f;
static constexpr float CTRL_H  = 40.0f;

void LeaderboardPanel::setup(HudManager& hud, graphic::IRenderer& renderer,
                              graphic::ITextureLoader& loader)
{
    _hud = &hud;
    static constexpr const char* MEDAL_PATHS[4] = {
        "assets/images/medals/Gold-Medal.png",
        "assets/images/medals/Silver-Medal.png",
        "assets/images/medals/Bronze-Medal.png",
        "assets/images/medals/participation_award.jpg"
    };
    for (int i = 0; i < 4; ++i) {
        try {
            auto texData = loader.loadFromFile(MEDAL_PATHS[i]);
            _medalTex[i] = renderer.uploadTexture(texData);
        } catch (...) {
            _log.warn("Failed to load medal texture ", i);
        }
    }
    spawnControls(hud);
}

void LeaderboardPanel::spawnControls(HudManager& hud)
{
    _ctrlProvider = std::make_shared<LeaderboardControlProvider>();

    auto entity = EntityBuilder(hud, LEADERBOARD_CTRL_ID, "leaderboard_controls")
        .hud().rect({0, 0}, {420, CTRL_H})
        .hud().container(_ctrlProvider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Horizontal, 8.0f)
        .hud().background(true, {10, 10, 20, 160}, {60, 60, 120, 180})
        .hud().anchor(graphic::Anchor::TopLeft)
        .hud().anchorOffset({LEFT, TOP})
        .hud().boxSize({420.0f, CTRL_H})
        .hud().hidden()
        .build();

    _ctrlContainer = entity->getBehavior<behavior::HudContainerBehavior>();
    if (_ctrlContainer) _ctrlContainer->setAnimationEnabled(false);
}

void LeaderboardPanel::onTeamAdded(const std::string& name, graphic::Color4b color)
{
    _teamColors[name] = color;
    _lbStore.addTeam(name);
    if (_hud) spawnEntry(*_hud, name, color);
    recompute();
}

void LeaderboardPanel::onPlayerAdded(uint32_t id, const std::string& team, int level)
{
    _lbStore.setPlayerTeam(id, team);
    if (level > 1)
        _lbStore.updateLevel(id, level);
    recompute();
}

void LeaderboardPanel::onPlayerRemoved(uint32_t id)
{
    _lbStore.removePlayer(id);
    recompute();
}

void LeaderboardPanel::onPlayerLevelChanged(uint32_t id, int level)
{
    _lbStore.updateLevel(id, level);
    recompute();
}

void LeaderboardPanel::toggle()
{
    _visible = !_visible;
    recompute();
}

std::vector<TeamLeaderboardStore::PlayerDetailStat> LeaderboardPanel::getPlayersForTeam(
    const std::string& name) const
{
    return _lbStore.getPlayersForTeam(name);
}

void LeaderboardPanel::setOnDetailsClick(const std::string& team, std::function<void()> cb)
{
    _detailCallbacks[team] = std::move(cb);
    auto it = _providers.find(team);
    if (it != _providers.end() && it->second)
        it->second->setOnDetailsClick(_detailCallbacks[team]);
}

void LeaderboardPanel::setVotedTeam(const std::string& team)
{
    // clear old voted team
    if (!_votedTeam.empty()) {
        auto pit = _providers.find(_votedTeam);
        if (pit != _providers.end() && pit->second)
            pit->second->setVoted(false);
        auto cit = _containers.find(_votedTeam);
        if (cit != _containers.end() && cit->second)
            cit->second->setBorderWidth(1.5f);
    }

    _votedTeam = team;

    bool anyVote = !_votedTeam.empty();
    for (auto& [t, p] : _providers)
        if (p) p->setAnyVoteCast(anyVote);

    if (!_votedTeam.empty()) {
        auto pit = _providers.find(_votedTeam);
        if (pit != _providers.end() && pit->second)
            pit->second->setVoted(true);
        auto cit = _containers.find(_votedTeam);
        if (cit != _containers.end() && cit->second)
            cit->second->setBorderWidth(3.5f);
    }
}

void LeaderboardPanel::setOnVoteClick(const std::string& team, std::function<void()> cb)
{
    _voteCallbacks[team] = std::move(cb);
    auto it = _providers.find(team);
    if (it != _providers.end() && it->second)
        it->second->setOnVoteClick(_voteCallbacks[team]);
}

void LeaderboardPanel::setOnWorldInfoClick(std::function<void()> cb)
{
    _onWorldInfoClick = std::move(cb);
    if (_ctrlProvider)
        _ctrlProvider->setOnWorldInfo(_onWorldInfoClick);
}

void LeaderboardPanel::setOnSelectTeamClick(const std::string& team, std::function<void()> cb)
{
    _selectTeamCallbacks[team] = std::move(cb);
    auto it = _providers.find(team);
    if (it != _providers.end() && it->second)
        it->second->setOnSelectTeamClick(_selectTeamCallbacks[team]);
}

void LeaderboardPanel::spawnEntry(HudManager& hud, const std::string& team,
                                   graphic::Color4b color)
{
    if (_entityIds.count(team)) return;

    graphic::EntityID id = LEADERBOARD_BASE_ID + static_cast<graphic::EntityID>(_entityIds.size());
    _entityIds[team] = id;

    auto provider = std::make_shared<TeamLeaderboardProvider>(&_lbStore, team);
    provider->setMedalTextures(_medalTex[0], _medalTex[1], _medalTex[2], _medalTex[3], 64.0f, 64.0f);
    if (_detailCallbacks.count(team))
        provider->setOnDetailsClick(_detailCallbacks[team]);
    if (_selectTeamCallbacks.count(team))
        provider->setOnSelectTeamClick(_selectTeamCallbacks[team]);
    if (_voteCallbacks.count(team))
        provider->setOnVoteClick(_voteCallbacks[team]);
    if (!_votedTeam.empty()) {
        provider->setAnyVoteCast(true);
        if (_votedTeam == team)
            provider->setVoted(true);
    }
    _providers[team] = provider;

    graphic::Color4b bg     = {10, 10, 20, 180};
    graphic::Color4b border = color;
    border.a = 200;

    auto entity = EntityBuilder(hud, id, "leaderboard_entry")
        .hud().rect({0, 0}, {420, 140})
        .hud().container(provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::MediaObjectHButtons, 4.0f, 3)
        .hud().background(true, bg, border)
        .hud().anchor(graphic::Anchor::TopLeft)
        .hud().anchorOffset({LEFT, TOP})
        .hud().boxSize({420.0f, 140.0f})
        .hud().hidden()
        .build();

    _containers[team] = entity->getBehavior<behavior::HudContainerBehavior>();
    if (_containers[team]) _containers[team]->setAnimationEnabled(false);
}

void LeaderboardPanel::scrollPrev()
{
    if (_scrollOffset > 0) {
        --_scrollOffset;
        recompute();
    }
}

void LeaderboardPanel::scrollNext()
{
    auto ranked = _lbStore.getRankedTeams();
    int total = static_cast<int>(ranked.size());
    if (_scrollOffset + MAX_VISIBLE < total) {
        ++_scrollOffset;
        recompute();
    }
}

void LeaderboardPanel::recompute()
{
    auto ranked = _lbStore.getRankedTeams();
    int total = static_cast<int>(ranked.size());

    // clamp offset
    int maxOffset = std::max(0, total - MAX_VISIBLE);
    if (_scrollOffset > maxOffset) _scrollOffset = maxOffset;

    for (int i = 0; i < total; ++i) {
        auto cit = _containers.find(ranked[i].name);
        if (cit == _containers.end() || !cit->second) continue;

        int slot = i - _scrollOffset;
        bool inWindow = slot >= 0 && slot < MAX_VISIBLE;

        if (_visible && inWindow) {
            cit->second->setAnchorOffset({LEFT, TOP + static_cast<float>(slot) * ROW_H});
            cit->second->setVisible(true);
        } else {
            cit->second->setVisible(false);
        }
    }

    if (_ctrlContainer) {
        if (_visible) {
            int visibleCount = std::min(total - _scrollOffset, MAX_VISIBLE);
            float ctrlY = TOP + static_cast<float>(visibleCount) * ROW_H + 4.0f;
            _ctrlContainer->setAnchorOffset({LEFT, ctrlY});
            _ctrlProvider->update(
                _scrollOffset, total, MAX_VISIBLE,
                [this]() { scrollPrev(); },
                [this]() { scrollNext(); });
            if (_onWorldInfoClick)
                _ctrlProvider->setOnWorldInfo(_onWorldInfoClick);
            _ctrlContainer->setVisible(true);
        } else {
            _ctrlContainer->setVisible(false);
        }
    }
}

} // namespace zappy
