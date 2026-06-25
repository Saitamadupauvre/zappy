#pragma once
#include "hud/IHudProvider.hpp"
#include "graphic/Types.hpp"
#include <array>
#include <cstdint>
#include <string>

class ResourceInfoProvider : public behavior::hud::IHudProvider {
public:
    void updateData(int x, int y, int type, int count = 0) {
        _x = x; _y = y; _type = type; _count = count;
        _hasData = true;
        markDirty();
    }

    void clear() { _hasData = false; markDirty(); }

    void setImage(graphic::TextureHandle tex, float w = 64.0f, float h = 64.0f) {
        _tex = tex; _imgW = w; _imgH = h;
        _hasImage = tex.id != 0;
        markDirty();
    }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override {
        if (!_dirty) return _cache;
        _dirty = false;
        _cache.clear();
        if (!_hasData) return _cache;
        static constexpr std::array<const char*, 7> NAMES = {
            "Food", "Linemate", "Deraumere", "Sibur", "Mendiane", "Phiras", "Thystame"
        };
        const char* name = (_type >= 0 && _type < 7) ? NAMES[_type] : "Unknown";
        if (_hasImage)
            _cache.push_back({ behavior::hud::ImageData{_tex, _imgW, _imgH} });
        _cache.push_back({ behavior::hud::TextData{name,                                                                    16.0f, {255, 255, 255, 255}} });
        _cache.push_back({ behavior::hud::TextData{"Pos: " + std::to_string(_x) + ", " + std::to_string(_y),               13.0f, {180, 210, 255, 255}} });
        _cache.push_back({ behavior::hud::TextData{"Count: " + std::to_string(_count),                                     13.0f, {180, 255, 180, 255}} });
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    int  _x = 0, _y = 0, _type = 0, _count = 0;
    bool _hasData = false;

    graphic::TextureHandle _tex{};
    float _imgW = 64.0f, _imgH = 64.0f;
    bool  _hasImage = false;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() { _dirty = true; ++_version; }
};
