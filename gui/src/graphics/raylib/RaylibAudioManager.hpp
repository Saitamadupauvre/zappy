#pragma once

#include "audio/IAudioManager.hpp"
#include <array>
#include <string>
#include <cstddef>
#include <raylib.h>

namespace graphic::raylib {

struct SoundEntry {
    std::string path;
    float       gain     = 1.0f;  // relative to global sound volume
    float       cooldown = 0.0f;  // min seconds between plays of this kind
};

struct AudioConfig {
    static constexpr std::size_t NUM_KINDS = 5;

    // Indexed by audio::SoundKind cast to size_t
    std::array<SoundEntry, NUM_KINDS> sounds = {{
        { "assets/sound/cartoon_walk.mp3",      2.0f, 0.25f }, // WALK
        { "assets/sound/pickup.mp3",             1.0f, 0.10f }, // PICKUP
        { "assets/sound/incantation.mp3",        1.75f, 2.00f }, // INCANTATION
        { "assets/sound/firework_explosion.mp3", 1.0f, 0.05f }, // EXPLOSION
        { "assets/sound/broadcast.wav",            1.6f, 0.30f }, // BROADCAST
    }};

    std::string musicPath    = "assets/sound/music.wav";
    float       maxDist      = 30.f;
    int         maxConcurrent = 8;
};

class RaylibAudioManager : public audio::IAudioManager {
public:
    explicit RaylibAudioManager(AudioConfig config = {});
    ~RaylibAudioManager() override { destroy(); }

    void init()                                               override;
    void update(float dt)                                     override;
    void playAt(audio::SoundKind kind, graphic::Vector3f pos) override;
    void stopSound(audio::SoundKind kind)                     override;
    void setListenerPos(graphic::Vector3f pos)                override;
    void setSoundVolume(float vol)                            override;
    void setMusicVolume(float vol)                            override;
    void destroy()                                            override;

private:
    AudioConfig _cfg;

    bool  _initialized = false;
    float _soundVolume = 0.8f;
    float _musicVolume = 0.5f;
    graphic::Vector3f _listenerPos{};

    std::array<Sound, AudioConfig::NUM_KINDS> _sounds{};
    Music _music{};

    std::array<float, AudioConfig::NUM_KINDS> _cooldown{};
    int _activeCount = 0;
};

} // namespace graphic::raylib
