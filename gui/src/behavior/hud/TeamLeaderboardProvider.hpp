#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "graphic/Types.hpp"
#include "world/TeamLeaderboardStore.hpp"
#include <cstdint>
#include <functional>
#include <string>

class TeamLeaderboardProvider : public behavior::hud::IHudProvider {
public:
    TeamLeaderboardProvider(zappy::TeamLeaderboardStore* store, std::string team)
        : _store(store), _team(std::move(team)) {}

    void setOnDetailsClick(std::function<void()> cb) { _onDetailsClick = std::move(cb); markDirty(); }
    void setOnSelectTeamClick(std::function<void()> cb) { _onSelectTeamClick = std::move(cb); markDirty(); }
    void setOnVoteClick(std::function<void()> cb) { _onVoteClick = std::move(cb); markDirty(); }
    void setVoted(bool voted) { _isVoted = voted; markDirty(); }

    void setTexture(graphic::TextureHandle tex, float w = 64.0f, float h = 64.0f)
    {
        _tex = tex; _texW = w; _texH = h;
        markDirty();
    }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override
    {
        uint64_t storeV = _store ? _store->getVersion() : 0;
        if (!_dirty && storeV == _lastStoreVersion) return _cache;
        _dirty = false;
        _lastStoreVersion = storeV;
        ++_version;

        _cache.clear();
        const auto& ranked = _store->getRankedTeams();
        int rank = 1;
        int maxLvl = 0;
        int cnt = 0;
        for (const auto& s : ranked) {
            if (s.name == _team) { maxLvl = s.maxLevel; cnt = s.playerCount; break; }
            ++rank;
        }

        if (_tex.id != 0)
            _cache.push_back({ behavior::hud::ImageData{_tex, _texW, _texH} });
        _cache.push_back({ behavior::hud::TextData{
            "#" + std::to_string(rank) + "  " + _team,
            22.0f, {255, 255, 255, 255}} });
        if (_isVoted)
            _cache.push_back({ behavior::hud::TextData{
                "VOTED", 13.0f, {255, 220, 50, 255}} });
        _cache.push_back({ behavior::hud::TextData{
            "Max Level: " + std::to_string(maxLvl),
            17.0f, {255, 210, 80, 255}} });
        _cache.push_back({ behavior::hud::TextData{
            "Players: " + std::to_string(cnt),
            17.0f, {140, 200, 255, 255}} });
        _cache.push_back({ behavior::hud::ButtonData{
            "Details", 13.0f, 90.0f, 24.0f,
            {255, 255, 255, 255}, {50, 80, 160, 210}, {80, 120, 220, 230},
            _onDetailsClick} });
        _cache.push_back({ behavior::hud::ButtonData{
            "Select Team", 13.0f, 90.0f, 24.0f,
            {255, 255, 255, 255}, {100, 50, 160, 210}, {140, 80, 220, 230},
            _onSelectTeamClick} });
        _cache.push_back({ behavior::hud::ButtonData{
            _isVoted ? "Unvote" : "Vote", 13.0f, 90.0f, 24.0f,
            {255, 255, 255, 255},
            _isVoted ? graphic::Color4b{160, 100, 20, 210} : graphic::Color4b{30, 120, 50, 210},
            _isVoted ? graphic::Color4b{220, 150, 30, 230} : graphic::Color4b{50, 180, 80, 230},
            _onVoteClick} });
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    zappy::TeamLeaderboardStore* _store;
    std::string                  _team;
    graphic::TextureHandle        _tex{};
    float                         _texW = 64.0f;
    float                         _texH = 64.0f;
    std::function<void()>         _onDetailsClick;
    std::function<void()>         _onSelectTeamClick;
    std::function<void()>         _onVoteClick;
    bool                          _isVoted = false;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty            = true;
    mutable uint64_t _version          = 1;
    mutable uint64_t _lastStoreVersion = 0;

    void markDirty() { _dirty = true; ++_version; }
};
