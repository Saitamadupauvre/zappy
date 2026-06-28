#pragma once
#include "hud/IHudProvider.hpp"
#include "graphic/Types.hpp"
#include "i18n/I18n.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

class ResourceInfoProvider : public behavior::hud::IHudProvider {
public:
    void setResourceTextures(const std::array<graphic::TextureHandle, 7>& textures)
    {
        _resTex = textures;
        markDirty();
    }

    void updateData(int x, int y, int type, int count = 0) {
        _x = x; _y = y; _type = type; _count = count;
        _hasData = true;
        if (type >= 0 && type < 7) {
            _tex = _resTex[type];
            _hasImage = _tex.id != 0;
        } else {
            _hasImage = false;
        }
        markDirty();
    }

    void clear() { _hasData = false; _hasImage = false; markDirty(); }

    void setImage(graphic::TextureHandle tex, float w = 64.0f, float h = 64.0f) {
        _tex = tex; _imgW = w; _imgH = h;
        _hasImage = tex.id != 0;
        markDirty();
    }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override {
        if (!_dirty && i18n::I18n::getVersion() == _i18nVersion) return _cache;
        _i18nVersion = i18n::I18n::getVersion();
        _dirty = false;
        _cache.clear();
        if (!_hasData) return _cache;
        static constexpr std::array<const char*, 7> KEYS = {
            i18n::key::FOOD, i18n::key::LINEMATE, i18n::key::DERAUMERE,
            i18n::key::SIBUR, i18n::key::MENDIANE, i18n::key::PHIRAS, i18n::key::THYSTAME
        };
        const char* name = (_type >= 0 && _type < 7) ? i18n::tr(KEYS[_type]) : i18n::tr(i18n::key::UNKNOWN);
        if (_hasImage)
            _cache.push_back({ behavior::hud::ImageData{_tex, _imgW, _imgH} });
        _cache.push_back({ behavior::hud::TextData{name, 16.0f, {255, 255, 255, 255}} });
        _cache.push_back({ behavior::hud::TextData{std::string(i18n::tr(i18n::key::POS)) + ": " + std::to_string(_x) + ", " + std::to_string(_y), 13.0f, {180, 210, 255, 255}} });
        _cache.push_back({ behavior::hud::TextData{std::string(i18n::tr(i18n::key::COUNT)) + ": " + std::to_string(_count), 13.0f, {180, 255, 180, 255}} });
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    int  _x = 0, _y = 0, _type = 0, _count = 0;
    bool _hasData = false;

    std::array<graphic::TextureHandle, 7> _resTex{};
    graphic::TextureHandle _tex{};
    float _imgW = 64.0f, _imgH = 64.0f;
    bool  _hasImage = false;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty       = true;
    mutable uint64_t _version     = 1;
    mutable uint64_t _i18nVersion = 0;

    void markDirty() { _dirty = true; ++_version; }
};
