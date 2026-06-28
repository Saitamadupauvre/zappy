#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "graphic/Types.hpp"
#include "world/TeamLeaderboardStore.hpp"
#include "i18n/I18n.hpp"
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
    void setAnyVoteCast(bool any) { _anyVoteCast = any; markDirty(); }

    void setMedalTextures(graphic::TextureHandle gold, graphic::TextureHandle silver,
                          graphic::TextureHandle bronze, graphic::TextureHandle participation,
                          float w = 64.0f, float h = 64.0f)
    {
        _medalTex[0] = gold; _medalTex[1] = silver; _medalTex[2] = bronze;
        _medalTex[3] = participation;
        _texW = w; _texH = h;
        markDirty();
    }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override
    {
        uint64_t storeV = _store ? _store->getVersion() : 0;
        uint64_t i18nV  = i18n::I18n::getVersion();
        if (!_dirty && storeV == _lastStoreVersion && i18nV == _i18nVersion) return _cache;
        _i18nVersion = i18nV;
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

        int texIdx = (rank >= 1 && rank <= 3) ? rank - 1 : 3;
        if (_medalTex[texIdx].id != 0)
            _cache.push_back({ behavior::hud::ImageData{_medalTex[texIdx], _texW, _texH} });
        _cache.push_back({ behavior::hud::TextData{
            _team,
            22.0f, {255, 255, 255, 255}} });
        if (_isVoted)
            _cache.push_back({ behavior::hud::TextData{
                i18n::tr(i18n::key::VOTED), 13.0f, {255, 220, 50, 255}} });
        _cache.push_back({ behavior::hud::TextData{
            std::string(i18n::tr(i18n::key::MAX_LEVEL)) + ": " + std::to_string(maxLvl),
            17.0f, {255, 210, 80, 255}} });
        _cache.push_back({ behavior::hud::TextData{
            std::string(i18n::tr(i18n::key::PLAYERS)) + ": " + std::to_string(cnt),
            17.0f, {140, 200, 255, 255}} });
        _cache.push_back({ behavior::hud::ButtonData{
            i18n::tr(i18n::key::DETAILS), 13.0f, 90.0f, 24.0f,
            {255, 255, 255, 255}, {50, 80, 160, 210}, {80, 120, 220, 230},
            _onDetailsClick} });
        _cache.push_back({ behavior::hud::ButtonData{
            i18n::tr(i18n::key::SELECT_TEAM), 13.0f, 90.0f, 24.0f,
            {255, 255, 255, 255}, {100, 50, 160, 210}, {140, 80, 220, 230},
            _onSelectTeamClick} });
        _cache.push_back({ behavior::hud::ButtonData{
            _isVoted ? i18n::tr(i18n::key::VOTED) : i18n::tr(i18n::key::VOTE), 13.0f, 90.0f, 24.0f,
            {255, 255, 255, 255},
            _isVoted ? graphic::Color4b{160, 100, 20, 210} : graphic::Color4b{30, 120, 50, 210},
            _isVoted ? graphic::Color4b{220, 150, 30, 230} : graphic::Color4b{50, 180, 80, 230},
            _onVoteClick,
            _anyVoteCast} });
        return _cache;
    }

    uint64_t getVersion() const override {
        uint64_t sv = _store ? _store->getVersion() : 0;
        if (sv != _lastStoreVersion) {
            ++_version;
            _lastStoreVersion = sv;
            _dirty = true;
        }
        return _version;
    }

private:
    zappy::TeamLeaderboardStore* _store;
    std::string                  _team;
    graphic::TextureHandle        _medalTex[4]{};
    float                         _texW = 64.0f;
    float                         _texH = 64.0f;
    std::function<void()>         _onDetailsClick;
    std::function<void()>         _onSelectTeamClick;
    std::function<void()>         _onVoteClick;
    bool                          _isVoted     = false;
    bool                          _anyVoteCast = false;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty            = true;
    mutable uint64_t _version          = 1;
    mutable uint64_t _lastStoreVersion = 0;
    mutable uint64_t _i18nVersion      = 0;

    void markDirty() { _dirty = true; ++_version; }
};
