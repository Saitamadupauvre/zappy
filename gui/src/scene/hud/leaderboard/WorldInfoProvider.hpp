#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "parser/Resources/Resources.hpp"
#include <array>
#include <cstdint>

class WorldInfoProvider : public behavior::hud::IHudProvider {
public:
    struct Stats {
        zappy::Resources totalResources;
        std::array<int, 8> playersPerLevel{};
        int totalPlayers = 0;
    };

    void update(const Stats& stats)
    {
        _stats = stats;
        markDirty();
    }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override
    {
        if (!_dirty) return _cache;
        _dirty = false;
        _cache.clear();

        graphic::Color4b res  = {140, 220, 255, 255};
        graphic::Color4b lvl  = {180, 255, 180, 255};
        graphic::Color4b dim  = {160, 160, 160, 200};

        auto fmt = [](int v) { return std::to_string(v); };

        _cache = {
            { behavior::hud::TextData{"--- World Resources ---", 12.0f, dim} },
            { behavior::hud::TextData{"Food:       " + fmt(_stats.totalResources.food),      13.0f, res} },
            { behavior::hud::TextData{"Linemate:   " + fmt(_stats.totalResources.linemate),  13.0f, res} },
            { behavior::hud::TextData{"Deraumere:  " + fmt(_stats.totalResources.deraumere), 13.0f, res} },
            { behavior::hud::TextData{"Sibur:      " + fmt(_stats.totalResources.sibur),     13.0f, res} },
            { behavior::hud::TextData{"Mendiane:   " + fmt(_stats.totalResources.mendiane),  13.0f, res} },
            { behavior::hud::TextData{"Phiras:     " + fmt(_stats.totalResources.phiras),    13.0f, res} },
            { behavior::hud::TextData{"Thystame:   " + fmt(_stats.totalResources.thystame),  13.0f, res} },
            { behavior::hud::TextData{"--- Players (" + fmt(_stats.totalPlayers) + ") ---", 12.0f, dim} },
        };
        for (int i = 0; i < 8; ++i) {
            _cache.push_back({ behavior::hud::TextData{
                "Level " + fmt(i + 1) + ": " + fmt(_stats.playersPerLevel[i]) + " player(s)",
                13.0f, lvl
            }});
        }
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    Stats _stats;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() { _dirty = true; ++_version; }
};
