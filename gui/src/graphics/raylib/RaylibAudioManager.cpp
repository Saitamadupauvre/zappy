#include "RaylibAudioManager.hpp"
#include <algorithm>
#include <cmath>

namespace graphic::raylib {

static float dist3(graphic::Vector3f a, graphic::Vector3f b)
{
    float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

RaylibAudioManager::RaylibAudioManager(AudioConfig config)
    : _cfg(std::move(config))
{}

void RaylibAudioManager::init()
{
    if (_initialized) return;
    InitAudioDevice();

    for (std::size_t i = 0; i < AudioConfig::NUM_KINDS; ++i)
        _sounds[i] = LoadSound(_cfg.sounds[i].path.c_str());

    _music = LoadMusicStream(_cfg.musicPath.c_str());
    _music.looping = true;
    SetMusicVolume(_music, _musicVolume);
    PlayMusicStream(_music);
    _initialized = true;
}

void RaylibAudioManager::update(float dt)
{
    if (!_initialized) return;
    UpdateMusicStream(_music);

    for (auto& c : _cooldown)
        c = std::max(0.f, c - dt);

    _activeCount = 0;
    for (std::size_t i = 0; i < AudioConfig::NUM_KINDS; ++i)
        if (IsSoundPlaying(_sounds[i])) _activeCount++;
}

void RaylibAudioManager::playAt(audio::SoundKind kind, graphic::Vector3f pos)
{
    if (!_initialized) return;

    auto idx = static_cast<std::size_t>(kind);
    if (_cooldown[idx] > 0.f) return;
    if (_activeCount >= _cfg.maxConcurrent) return;

    float d   = dist3(pos, _listenerPos);
    float vol = _soundVolume * _cfg.sounds[idx].gain
                * std::max(0.f, 1.f - d / _cfg.maxDist);
    vol = std::min(vol, 1.f);
    if (vol < 0.01f) return;

    SetSoundVolume(_sounds[idx], vol);
    PlaySound(_sounds[idx]);

    _cooldown[idx] = _cfg.sounds[idx].cooldown;
    _activeCount++;
}

void RaylibAudioManager::stopSound(audio::SoundKind kind)
{
    if (!_initialized) return;
    StopSound(_sounds[static_cast<std::size_t>(kind)]);
}

void RaylibAudioManager::setListenerPos(graphic::Vector3f pos)
{
    _listenerPos = pos;
}

void RaylibAudioManager::setSoundVolume(float vol)
{
    _soundVolume = std::clamp(vol, 0.f, 1.f);
}

void RaylibAudioManager::setMusicVolume(float vol)
{
    _musicVolume = std::clamp(vol, 0.f, 1.f);
    if (_initialized)
        SetMusicVolume(_music, _musicVolume);
}

void RaylibAudioManager::destroy()
{
    if (!_initialized) return;
    StopMusicStream(_music);
    UnloadMusicStream(_music);
    for (std::size_t i = 0; i < AudioConfig::NUM_KINDS; ++i)
        UnloadSound(_sounds[i]);
    CloseAudioDevice();
    _initialized = false;
}

} // namespace graphic::raylib
