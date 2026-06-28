#include "VideoSection.hpp"

namespace zappy {

std::vector<behavior::hud::HudElement> VideoSection::getHudElements() const
{
    std::vector<behavior::hud::HudElement> elems;

    {
        std::vector<std::string> labels;
        labels.reserve(std::size(FPS_OPTIONS));
        for (auto& [label, _] : FPS_OPTIONS)
            labels.push_back(label);

        elems.push_back({behavior::hud::SelectData{
            .label        = i18n::tr(i18n::key::FRAME_RATE),
            .options      = std::move(labels),
            .currentIndex = _fpsIdx,
            .isOpen       = _fpsOpen,
            .width        = 260.f,
            .rowHeight    = 28.f,
            .fontSize     = 12.f,
            .onToggle     = [this]() { _fpsOpen = !_fpsOpen; },
            .onSelect     = [this](int idx) {
                _fpsIdx  = idx;
                _fpsOpen = false;
                if (_onFpsChange)
                    _onFpsChange(FPS_OPTIONS[idx].second);
            },
        }});
    }

    elems.push_back({behavior::hud::RectData{260.f, 1.f, {80,90,130,80}, {80,90,130,80}}});

    {
        std::vector<std::string> labels;
        labels.reserve(std::size(RES_OPTIONS));
        for (auto& [label, w, h] : RES_OPTIONS) {
            (void)w; (void)h;
            labels.push_back(label);
        }

        elems.push_back({behavior::hud::SelectData{
            .label        = i18n::tr(i18n::key::RESOLUTION),
            .options      = std::move(labels),
            .currentIndex = _resIdx,
            .isOpen       = _resOpen && !_fullscreen,
            .width        = 260.f,
            .rowHeight    = 28.f,
            .fontSize     = 12.f,
            .onToggle     = [this]() { if (!_fullscreen) _resOpen = !_resOpen; },
            .onSelect     = [this](int idx) {
                _resIdx  = idx;
                _resOpen = false;
                auto& [label, w, h] = RES_OPTIONS[idx];
                (void)label;
                if (_onResolution) _onResolution(w, h);
            },
        }});
    }

    elems.push_back({behavior::hud::RectData{260.f, 1.f, {80,90,130,80}, {80,90,130,80}}});

    elems.push_back({behavior::hud::ToggleData{
        .label    = i18n::tr(i18n::key::FULLSCREEN),
        .value    = _fullscreen,
        .width    = 260.f,
        .height   = 32.f,
        .fontSize = 12.f,
        .onToggle = [this](bool v) {
            _fullscreen = v;
            if (v) _resOpen = false;
            if (_onFullscreen) _onFullscreen(v);
        },
    }});


    elems.push_back({behavior::hud::RectData{260.f, 1.f, {80,90,130,80}, {80,90,130,80}}});

    elems.push_back({behavior::hud::ToggleData{
        .label    = i18n::tr(i18n::key::SHOW_FPS),
        .value    = _fpsOverlay,
        .width    = 260.f,
        .height   = 32.f,
        .fontSize = 12.f,
        .onToggle = [this](bool v) {
            _fpsOverlay = v;
            if (_onFpsOverlay) _onFpsOverlay(v);
        },
    }});

    elems.push_back({behavior::hud::RectData{260.f, 1.f, {80,90,130,80}, {80,90,130,80}}});

    elems.push_back({behavior::hud::TextData{i18n::tr(i18n::key::FIELD_OF_VIEW), 11.f, {190, 200, 230, 200}}});
    elems.push_back({behavior::hud::SliderData{
        .min      = 45.f,
        .max      = 120.f,
        .value    = _fov,
        .width    = 260.f,
        .height   = 14.f,
        .onChange = [this](float v) { _fov = v; },
        .onRelease= [this](float v) {
            _fov = v;
            if (_onFov) _onFov(v);
        },
    }});

    return elems;
}

} // namespace zappy
