#include "GameDisplaySection.hpp"

namespace zappy {

std::vector<behavior::hud::HudElement> GameDisplaySection::getHudElements() const
{
    std::vector<behavior::hud::HudElement> elems;

    auto sep = [&]() {
        elems.push_back({behavior::hud::RectData{260.f, 1.f, {80,90,130,80}, {80,90,130,80}}});
    };

    elems.push_back({behavior::hud::ToggleData{
        .label    = i18n::tr(i18n::key::INCANTATION_FX),
        .value    = _incantation,
        .width    = 260.f,
        .height   = 32.f,
        .fontSize = 12.f,
        .onToggle = [this](bool v) {
            _incantation = v;
            if (_onIncantation) _onIncantation(v);
        },
    }});

    sep();

    elems.push_back({behavior::hud::ToggleData{
        .label    = i18n::tr(i18n::key::BROADCAST_CIRCLE),
        .value    = _broadcastCircle,
        .width    = 260.f,
        .height   = 32.f,
        .fontSize = 12.f,
        .onToggle = [this](bool v) {
            _broadcastCircle = v;
            if (_onBroadcast) _onBroadcast(v);
        },
    }});

    sep();

    elems.push_back({behavior::hud::ToggleData{
        .label    = i18n::tr(i18n::key::EGG_HATCH_ANIM),
        .value    = _eggHatchAnim,
        .width    = 260.f,
        .height   = 32.f,
        .fontSize = 12.f,
        .onToggle = [this](bool v) {
            _eggHatchAnim = v;
            if (_onEggHatch) _onEggHatch(v);
        },
    }});

    sep();

    elems.push_back({behavior::hud::ToggleData{
        .label    = i18n::tr(i18n::key::TEAM_COLOR_TAGS),
        .value    = _teamColorTags,
        .width    = 260.f,
        .height   = 32.f,
        .fontSize = 12.f,
        .onToggle = [this](bool v) {
            _teamColorTags = v;
            if (_onTeamColor) _onTeamColor(v);
        },
    }});

    sep();

    elems.push_back({behavior::hud::ToggleData{
        .label    = i18n::tr(i18n::key::GRASS),
        .value    = _grass,
        .width    = 260.f,
        .height   = 32.f,
        .fontSize = 12.f,
        .onToggle = [this](bool v) {
            _grass = v;
            if (_onGrass) _onGrass(v);
        },
    }});

    sep();

    {
        std::vector<std::string> labels;
        for (auto opt : SKY_OPTIONS)
            labels.push_back(opt);

        elems.push_back({behavior::hud::SelectData{
            .label        = i18n::tr(i18n::key::SKY),
            .options      = std::move(labels),
            .currentIndex = _skyIdx,
            .isOpen       = _skyOpen,
            .width        = 260.f,
            .rowHeight    = 28.f,
            .fontSize     = 12.f,
            .onToggle     = [this]() { _skyOpen = !_skyOpen; },
            .onSelect     = [this](int idx) {
                _skyIdx  = idx;
                _skyOpen = false;
                if (_onSkyMode) _onSkyMode(idx);
            },
        }});
    }

    return elems;
}

} // namespace zappy
