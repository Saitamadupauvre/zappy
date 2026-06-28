#pragma once

#include "ISettingsSection.hpp"
#include "graphic/Types.hpp"
#include "i18n/I18n.hpp"
#include <array>
#include <functional>
#include <span>

namespace zappy {

class LanguageSection : public ISettingsSection {
public:
    std::string sectionTitle() const override { return i18n::tr(i18n::key::SEC_LANGUAGE); }

    void setFlagTextures(std::span<const graphic::TextureHandle> flags)
    {
        for (std::size_t i = 0; i < std::min(flags.size(), _flags.size()); ++i)
            _flags[i] = flags[i];
    }

    std::vector<behavior::hud::HudElement> getHudElements() const override
    {
        std::vector<behavior::hud::HudElement> elems;
        elems.push_back({ behavior::hud::TextData{
            i18n::tr(i18n::key::LANGUAGE), 12.f, {200, 200, 220, 255}} });

        static constexpr int LANG_COUNT = 18;
        static constexpr int COLS = 4;
        static constexpr const char* LANG_NAMES[LANG_COUNT] = {
            "English",  "Francais", "Espanol",  "Chinese",   "Japanese",
            "Pirate",   "Korean",   "Arabic",   "Hindi",     "Portugues",
            "Deutsch",  "Russian",  "Italiano", "Turkce",    "Polski",
            "Dutch",    "Viet",     "Zimbabwe",
        };

        int cur = static_cast<int>(i18n::I18n::getLanguage());

        auto makeChild = [&](int i) -> behavior::hud::ImageButtonData {
            bool selected = (i == cur);
            int idx = i;
            return {
                .texture    = _flags[i],
                .width      = 64.f,
                .height     = 42.f,
                .tint       = selected ? graphic::Color4b{255, 255, 255, 255}
                                       : graphic::Color4b{180, 180, 180, 160},
                .hoverTint  = {255, 255, 255, 230},
                .opacity    = selected ? 1.f : 0.55f,
                .onClick    = [this, idx]() {
                    auto lang = static_cast<i18n::Language>(idx);
                    i18n::I18n::setLanguage(lang);
                    if (_onChange) _onChange(lang);
                },
                .label      = LANG_NAMES[i],
                .labelSize  = 8.f,
                .labelColor = selected ? graphic::Color4b{220, 220, 255, 255}
                                       : graphic::Color4b{160, 160, 180, 200},
            };
        };

        for (int i = 0; i < LANG_COUNT; i += COLS) {
            behavior::hud::HRowData row;
            row.gap = 12.f;
            for (int j = i; j < std::min(i + COLS, LANG_COUNT); ++j)
                row.children.push_back(makeChild(j));
            elems.push_back({ std::move(row) });
        }
        return elems;
    }

    void setOnLanguageChange(std::function<void(i18n::Language)> fn) { _onChange = std::move(fn); }

private:
    std::array<graphic::TextureHandle, 18> _flags{};
    std::function<void(i18n::Language)>   _onChange;
};

} // namespace zappy
