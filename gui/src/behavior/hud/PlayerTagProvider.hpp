#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "behavior/player/BroadcastBehavior.hpp"
#include "i18n/I18n.hpp"
#include <cstdint>

class PlayerTagProvider : public behavior::hud::IHudProvider {
public:
    explicit PlayerTagProvider(uint32_t playerId)
        : _playerId(playerId) {}

    void setBroadcastBehavior(behavior::BroadcastBehavior* b) { _broadcast = b; }
    void setTeamColor(graphic::Color4b c) { _teamColor = c; invalidate(); }
    void setShowTeamColor(bool v)         { _showTeamColor = v; invalidate(); }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override {
        bool broadcasting = _broadcast && _broadcast->isBroadcasting();
        uint64_t i18nV = i18n::I18n::getVersion();
        if (broadcasting == _lastBroadcast && !_dirty && i18nV == _i18nVersion) return _cache;
        _i18nVersion = i18nV;
        _lastBroadcast = broadcasting;
        _dirty = false;
        ++_version;

        graphic::Color4b nameColor = _showTeamColor ? _teamColor : graphic::Color4b{0, 0, 0, 255};
        _cache.clear();
        _cache.push_back({ behavior::hud::TextData{
            std::string(i18n::tr(i18n::key::PLAYER)) + " " + std::to_string(_playerId), 18.0f, nameColor} });
        if (broadcasting)
            _cache.push_back({ behavior::hud::TextData{"...", 14.0f, {80, 80, 80, 200}} });
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    void invalidate() { _dirty = true; }

    uint32_t                      _playerId;
    behavior::BroadcastBehavior*  _broadcast      = nullptr;
    graphic::Color4b              _teamColor       = {0, 0, 0, 255};
    bool                          _showTeamColor   = false;
    mutable bool                  _lastBroadcast   = false;
    mutable bool                  _dirty           = false;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable uint64_t _version     = 1;
    mutable uint64_t _i18nVersion = 0;
};
