#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "graphic/Types.hpp"
#include "i18n/I18n.hpp"
#include <cstdint>
#include <string>

class NotificationPopupProvider : public behavior::hud::IHudProvider {
public:
    void set(const std::string& title, const std::string& subtitle,
             graphic::Color4b titleColor = {255, 255, 255, 255})
    {
        _title      = title;
        _subtitle   = subtitle;
        _titleColor = titleColor;
        markDirty();
    }

    void setTexture(graphic::TextureHandle tex, float w = 64.f, float h = 64.f)
    {
        _tex = tex; _texW = w; _texH = h;
        markDirty();
    }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override
    {
        if (!_dirty) return _cache;
        _dirty = false;
        _cache.clear();
        if (_tex.id != 0)
            _cache.push_back({behavior::hud::ImageData{_tex, _texW, _texH}});
        _cache.push_back({behavior::hud::TextData{_title,    16.f, _titleColor}});
        _cache.push_back({behavior::hud::TextData{_subtitle, 13.f, {200, 200, 200, 255}}});
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    std::string            _title;
    std::string            _subtitle;
    graphic::Color4b       _titleColor{255, 255, 255, 255};
    graphic::TextureHandle _tex{};
    float                  _texW = 64.f;
    float                  _texH = 64.f;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() { _dirty = true; ++_version; }
};
