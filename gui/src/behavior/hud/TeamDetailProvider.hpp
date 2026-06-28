#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "i18n/I18n.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

struct PlayerDetailEntry {
    uint32_t    id;
    int         level;
    int         x;
    int         y;
};

class TeamDetailProvider : public behavior::hud::IHudProvider {
public:
    void setTeam(const std::string& team) { _team = team; markDirty(); }
    const std::string& getTeam() const { return _team; }

    void setPlayers(std::vector<PlayerDetailEntry> players) {
        _players = std::move(players);
        markDirty();
    }

    void setOnFollowClick(std::function<void(uint32_t)> cb) {
        _onFollow = std::move(cb);
        markDirty();
    }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override
    {
        if (!_dirty && i18n::I18n::getVersion() == _i18nVersion) return _cache;
        _i18nVersion = i18n::I18n::getVersion();
        _dirty = false;
        _cache.clear();

        if (_players.empty()) {
            _cache.push_back({ behavior::hud::TextData{i18n::tr(i18n::key::NO_PLAYERS), 14.0f, {160, 160, 160, 200}} });
            return _cache;
        }

        for (const auto& p : _players) {
            _cache.push_back({ behavior::hud::TextData{
                std::string(i18n::tr(i18n::key::PLAYER)) + " #" + std::to_string(p.id),
                15.0f, {255, 255, 255, 255}} });
            _cache.push_back({ behavior::hud::TextData{
                "  " + std::string(i18n::tr(i18n::key::LEVEL)) + ": " + std::to_string(p.level),
                13.0f, {255, 210, 80, 255}} });
            _cache.push_back({ behavior::hud::TextData{
                "  " + std::string(i18n::tr(i18n::key::POS)) + ": (" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")",
                13.0f, {140, 200, 255, 255}} });
            _cache.push_back({ behavior::hud::ButtonData{
                i18n::tr(i18n::key::FOLLOW), 13.0f, 80.0f, 22.0f,
                {255, 255, 255, 255}, {60, 120, 200, 220}, {90, 160, 255, 230},
                [this, id = p.id]() { if (_onFollow) _onFollow(id); }} });
            _cache.push_back({ behavior::hud::RectData{
                300.0f, 1.0f, {80, 80, 100, 180}, {80, 80, 100, 180}} });
        }
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    std::string                    _team;
    std::vector<PlayerDetailEntry> _players;
    std::function<void(uint32_t)>  _onFollow;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty       = true;
    mutable uint64_t _version     = 1;
    mutable uint64_t _i18nVersion = 0;

    void markDirty() { _dirty = true; ++_version; }
};
