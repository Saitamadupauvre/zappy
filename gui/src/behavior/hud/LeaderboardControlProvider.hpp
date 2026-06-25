#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include <cstdint>
#include <functional>
#include <string>

class LeaderboardControlProvider : public behavior::hud::IHudProvider {
public:
    void update(int offset, int total, int maxVisible,
                std::function<void()> onPrev, std::function<void()> onNext)
    {
        _offset     = offset;
        _total      = total;
        _maxVisible = maxVisible;
        _onPrev     = std::move(onPrev);
        _onNext     = std::move(onNext);
        markDirty();
    }

    void setOnWorldInfo(std::function<void()> cb) { _onWorldInfo = std::move(cb); markDirty(); }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override
    {
        if (!_dirty) return _cache;
        _dirty = false;
        _cache.clear();

        int last = std::min(_offset + _maxVisible, _total);
        std::string label = std::to_string(_offset + 1) + "-" +
                            std::to_string(last) + " / " + std::to_string(_total);

        graphic::Color4b activeBtn  = {60, 100, 180, 220};
        graphic::Color4b hoverBtn   = {90, 140, 220, 240};
        graphic::Color4b disabledBg = {40, 40, 40, 120};
        graphic::Color4b white      = {255, 255, 255, 255};
        graphic::Color4b dimWhite   = {180, 180, 180, 160};

        bool canPrev = _offset > 0;
        bool canNext = _offset + _maxVisible < _total;

        _cache.push_back({behavior::hud::ButtonData{
            "^ Prev", 12.0f, 90.0f, 24.0f,
            canPrev ? white : dimWhite,
            canPrev ? activeBtn : disabledBg,
            canPrev ? hoverBtn  : disabledBg,
            canPrev ? _onPrev : std::function<void()>{}}});
        _cache.push_back({behavior::hud::TextData{label, 13.0f, {200, 200, 200, 200}}});
        _cache.push_back({behavior::hud::ButtonData{
            "Next v", 12.0f, 90.0f, 24.0f,
            canNext ? white : dimWhite,
            canNext ? activeBtn : disabledBg,
            canNext ? hoverBtn  : disabledBg,
            canNext ? _onNext : std::function<void()>{}}});

        graphic::Color4b infoBtn   = {40, 120, 80, 220};
        graphic::Color4b infoHover = {60, 180, 110, 240};
        _cache.push_back({behavior::hud::ButtonData{
            "World Info", 12.0f, 90.0f, 24.0f,
            white, infoBtn, infoHover, _onWorldInfo}});
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    int _offset     = 0;
    int _total      = 0;
    int _maxVisible = 3;
    std::function<void()> _onPrev;
    std::function<void()> _onNext;
    std::function<void()> _onWorldInfo;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() { _dirty = true; ++_version; }
};
