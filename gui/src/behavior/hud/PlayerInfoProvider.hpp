#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "world/WorldTypes.hpp"
#include "i18n/I18n.hpp"
#include <cstdint>
#include <functional>
#include <string>

class PlayerInfoProvider : public behavior::hud::IHudProvider {
public:
    void setPlayer(const zappy::PlayerState& player) {
        _name      = std::string(i18n::tr(i18n::key::PLAYER)) + " " + std::to_string(player.id);
        _team      = player.team;
        _level     = player.level;
        _x         = player.x;
        _y         = player.y;
        _hasPlayer = true;
        markDirty();
    }

    void clear() { _hasPlayer = false; markDirty(); }

    void setOnChatClick(std::function<void()> cb) { _onChatClick = std::move(cb); markDirty(); }
    void setOnInventoryClick(std::function<void()> cb) { _onInventoryClick = std::move(cb); markDirty(); }
    void setVotedTeam(const std::string& team) { _votedTeam = team; markDirty(); }

    void setAvatar(graphic::TextureHandle tex, float w = 64.0f, float h = 64.0f) {
        _avatarTex = tex;
        _avatarW   = w;
        _avatarH   = h;
        _hasAvatar = tex.id != 0;
        markDirty();
    }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override {
        if (!_dirty && i18n::I18n::getVersion() == _i18nVersion) return _cache;
        _i18nVersion = i18n::I18n::getVersion();
        _dirty = false;
        _cache.clear();
        if (!_hasPlayer) return _cache;
        if (_hasAvatar)
            _cache.push_back({ behavior::hud::ImageData{_avatarTex, _avatarW, _avatarH} });
        _cache.push_back({ behavior::hud::TextData{_name,                    16.0f, {255, 255, 255, 255}} });
        _cache.push_back({ behavior::hud::TextData{std::string(i18n::tr(i18n::key::TEAM)) + ": " + _team, 13.0f, {180, 210, 255, 255}} });
        if (!_votedTeam.empty() && _team == _votedTeam)
            _cache.push_back({ behavior::hud::TextData{i18n::tr(i18n::key::FROM_VOTED_TEAM), 13.0f, {255, 220, 50, 255}} });
        _cache.push_back({ behavior::hud::TextData{std::string(i18n::tr(i18n::key::LEVEL)) + ": " + std::to_string(_level), 13.0f, {180, 255, 180, 255}} });
        _cache.push_back({ behavior::hud::TextData{std::string(i18n::tr(i18n::key::POS)) + ": (" + std::to_string(_x) + ", " + std::to_string(_y) + ")", 13.0f, {200, 200, 200, 255}} });
        _cache.push_back({ behavior::hud::ButtonData{i18n::tr(i18n::key::TEAM_CHAT), 14.0f, 130.0f, 28.0f,
            {255, 255, 255, 255}, {40, 110, 210, 220}, {70, 145, 255, 230},
            _onChatClick} });
        _cache.push_back({ behavior::hud::ButtonData{i18n::tr(i18n::key::INVENTORY), 14.0f, 130.0f, 28.0f,
            {255, 255, 255, 255}, {40, 150, 80, 220}, {70, 200, 110, 230},
            _onInventoryClick} });
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    bool        _hasPlayer = false;
    std::string _name;
    std::string _team;
    int         _level = 1;
    int         _x     = 0;
    int         _y     = 0;
    std::function<void()> _onChatClick;
    std::function<void()> _onInventoryClick;
    std::string           _votedTeam;
    graphic::TextureHandle _avatarTex{};
    float _avatarW = 64.0f;
    float _avatarH = 64.0f;
    bool  _hasAvatar = false;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty       = true;
    mutable uint64_t _version     = 1;
    mutable uint64_t _i18nVersion = 0;

    void markDirty() { _dirty = true; ++_version; }
};
