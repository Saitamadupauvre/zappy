#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include <cstdint>
#include <format>

class ClockProvider : public behavior::hud::IHudProvider {
public:
    void setUptime(unsigned long ticks, int timeUnit)
    {
        _baseSecs    = (timeUnit > 0) ? static_cast<double>(ticks) / timeUnit : 0.0;
        _localElapsed = 0.0;
        _hasData      = true;
        _lastDisplayedCs = UINT64_MAX;
        markDirty();
    }

    void setTimeUnit(int tu) { _timeUnit = tu; }

    void tick(float dt)
    {
        if (!_hasData) return;
        _localElapsed += static_cast<double>(dt);
        double total = _baseSecs + _localElapsed;
        uint64_t cs  = static_cast<uint64_t>(total * 100.0);
        if (cs != _lastDisplayedCs) {
            _lastDisplayedCs = cs;
            markDirty();
        }
    }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override
    {
        if (!_dirty) return _cache;
        _dirty = false;
        _cache.clear();

        if (!_hasData) {
            _cache.push_back({behavior::hud::TextData{"--:--:--", 30.f, {200, 220, 255, 220}}});
            return _cache;
        }

        double total = _baseSecs + _localElapsed;
        unsigned long totalCs = static_cast<unsigned long>(total * 100.0);
        unsigned long cs = totalCs % 100;
        unsigned long totalS  = totalCs / 100;
        unsigned long s  = totalS % 60;
        unsigned long m  = totalS / 60;

        _cache.push_back({behavior::hud::TextData{
            std::format("{:02}:{:02}:{:02}", m, s, cs), 30.f, {200, 220, 255, 220}}});
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    double   _baseSecs        = 0.0;
    double   _localElapsed    = 0.0;
    int      _timeUnit        = 1;
    bool     _hasData         = false;
    uint64_t _lastDisplayedCs = UINT64_MAX;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() const { ++_version; _dirty = true; }
};
