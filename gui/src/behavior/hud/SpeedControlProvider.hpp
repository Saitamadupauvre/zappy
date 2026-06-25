#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include <cstdint>
#include <functional>
#include <string>

class SpeedControlProvider : public behavior::hud::IHudProvider {
public:
    void setTimeUnit(int tu)
    {
        _timeUnit     = tu;
        _displayValue = static_cast<float>(tu);
        markDirty();
    }

    void setOnCommit(std::function<void(int)> cb) { _onCommit = std::move(cb); markDirty(); }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override
    {
        if (!_dirty) return _cache;
        _dirty = false;
        _cache.clear();

        std::string label = "Speed: " + std::to_string(static_cast<int>(_displayValue)) + "t/s";
        _cache.push_back({behavior::hud::TextData{label, 13.f, {200, 210, 255, 255}}});
        _cache.push_back({behavior::hud::SliderData{
            1.f, 100.f, _displayValue, 200.f, 14.f,
            [this](float v) { _displayValue = v; markDirty(); },
            [this](float v) {
                int tu = static_cast<int>(v + 0.5f);
                _displayValue = static_cast<float>(tu);
                if (tu != _timeUnit) {
                    _timeUnit = tu;
                    if (_onCommit) _onCommit(tu);
                }
                markDirty();
            }
        }});
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    mutable int   _timeUnit     = 1;
    mutable float _displayValue = 1.f;
    std::function<void(int)> _onCommit;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() const { ++_version; _dirty = true; }
};
