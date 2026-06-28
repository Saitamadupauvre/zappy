#pragma once

#include "ISettingsSection.hpp"
#include "i18n/I18n.hpp"
#include <functional>

namespace zappy {

class AudioSection : public ISettingsSection {
public:
    void setOnSoundVolume(std::function<void(float)> fn) { _onSoundVolume = std::move(fn); }
    void setOnMusicVolume(std::function<void(float)> fn) { _onMusicVolume = std::move(fn); }

    std::string sectionTitle() const override { return i18n::tr(i18n::key::SEC_AUDIO); }

    std::vector<behavior::hud::HudElement> getHudElements() const override
    {
        std::vector<behavior::hud::HudElement> elems;

        elems.push_back({behavior::hud::TextData{i18n::tr(i18n::key::SOUND_VOLUME), 11.f, {180, 190, 220, 200}}});
        elems.push_back({behavior::hud::SliderData{
            .min      = 0.f,
            .max      = 1.f,
            .value    = _soundVol,
            .width    = 260.f,
            .height   = 14.f,
            .onChange = [this](float v) { _soundVol = v; if (_onSoundVolume) _onSoundVolume(v); },
            .onRelease= [this](float v) { _soundVol = v; if (_onSoundVolume) _onSoundVolume(v); },
        }});

        elems.push_back({behavior::hud::TextData{i18n::tr(i18n::key::MUSIC_VOLUME), 11.f, {180, 190, 220, 200}}});
        elems.push_back({behavior::hud::SliderData{
            .min      = 0.f,
            .max      = 1.f,
            .value    = _musicVol,
            .width    = 260.f,
            .height   = 14.f,
            .onChange = [this](float v) { _musicVol = v; if (_onMusicVolume) _onMusicVolume(v); },
            .onRelease= [this](float v) { _musicVol = v; if (_onMusicVolume) _onMusicVolume(v); },
        }});

        return elems;
    }

private:
    mutable float _soundVol = 0.8f;
    mutable float _musicVol = 0.5f;

    std::function<void(float)> _onSoundVolume;
    std::function<void(float)> _onMusicVolume;
};

} // namespace zappy
