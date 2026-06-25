#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "parser/Resources/Resources.hpp"
#include <cstdint>
#include <cstdio>
#include <string>

class TeamStatsProvider : public behavior::hud::IHudProvider {
public:
    void update(const std::string& team, int playerCount, int maxLevel, float avgLevel,
                const zappy::Resources& total)
    {
        _team        = team;
        _playerCount = playerCount;
        _maxLevel    = maxLevel;
        _avgLevel    = avgLevel;
        _total       = total;
        _hasData     = true;
        markDirty();
    }

    void clear() { _hasData = false; markDirty(); }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override
    {
        if (!_dirty) return _cache;
        _dirty = false;
        _cache.clear();
        if (!_hasData) return _cache;

        auto fmt = [](int v) { return std::to_string(v); };
        auto fmtf = [](float v) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%.1f", v);
            return std::string(buf);
        };

        graphic::Color4b hdr  = {255, 210, 80,  255};
        graphic::Color4b stat = {200, 200, 200, 255};
        graphic::Color4b res  = {140, 220, 255, 255};

        _cache = {
            { behavior::hud::TextData{"Team: " + _team,                    16.0f, hdr} },
            { behavior::hud::TextData{"Players: " + fmt(_playerCount),     13.0f, stat} },
            { behavior::hud::TextData{"Max level: " + fmt(_maxLevel),      13.0f, stat} },
            { behavior::hud::TextData{"Avg level: " + fmtf(_avgLevel),     13.0f, stat} },
            { behavior::hud::TextData{"--- Resources held ---",            12.0f, {160,160,160,200}} },
            { behavior::hud::TextData{"Food:      " + fmt(_total.food),     13.0f, res} },
            { behavior::hud::TextData{"Linemate:  " + fmt(_total.linemate), 13.0f, res} },
            { behavior::hud::TextData{"Deraumere: " + fmt(_total.deraumere),13.0f, res} },
            { behavior::hud::TextData{"Sibur:     " + fmt(_total.sibur),    13.0f, res} },
            { behavior::hud::TextData{"Mendiane:  " + fmt(_total.mendiane), 13.0f, res} },
            { behavior::hud::TextData{"Phiras:    " + fmt(_total.phiras),   13.0f, res} },
            { behavior::hud::TextData{"Thystame:  " + fmt(_total.thystame), 13.0f, res} },
        };
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    bool             _hasData     = false;
    std::string      _team;
    int              _playerCount = 0;
    int              _maxLevel    = 0;
    float            _avgLevel    = 0.f;
    zappy::Resources _total;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() { _dirty = true; ++_version; }
};
