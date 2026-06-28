# Audio System

## Overview

Audio is abstracted behind `audio::IAudioManager` (implemented by `graphic::raylib::RaylibAudioManager`). It handles spatial sound effects and a background music track.

`GameEngine` owns the audio manager and provides it via `Locator::getAudio()` (if wired) or passes it directly to systems that need it.

## Sound kinds

`audio::SoundKind` enum:

| Value | File | Cooldown | Description |
|---|---|---|---|
| `WALK` | `cartoon_walk.mp3` | 0.25s | Player movement step |
| `PICKUP` | `pickup.mp3` | 0.10s | Resource collected / dropped |
| `INCANTATION` | `incantation.mp3` | 2.00s | Incantation start |
| `EXPLOSION` | `firework_explosion.mp3` | 0.05s | Incantation end explosion |
| `BROADCAST` | `broadcast.wav` | 0.30s | Player broadcast message |

## Playing a sound

```cpp
#include "audio/IAudioManager.hpp"

// Spatial sound — attenuated by distance from listener:
audioManager.playAt(audio::SoundKind::WALK, playerWorldPos);

// Stop a looping or one-shot sound early:
audioManager.stopSound(audio::SoundKind::INCANTATION);
```

`playAt` is a no-op if:
- The sound's cooldown has not elapsed since the last play.
- `_activeCount` has reached `maxConcurrent` (default: 8).
- Distance from listener exceeds `maxDist` (default: 30 world units).

## Listener position

Update the listener position each frame to match the camera:

```cpp
audioManager.setListenerPos(cameraWorldPos);
```

Volume falls off linearly from 0 to `maxDist`.

## Volume control

```cpp
audioManager.setSoundVolume(0.8f);  // 0.0–1.0
audioManager.setMusicVolume(0.5f);  // 0.0–1.0
```

`SettingsPanel` exposes sliders that call these methods via callback.

## Music

Background music (`assets/sound/music.wav`) is loaded and streamed by `RaylibAudioManager::init()`. It loops automatically. `update(dt)` must be called each frame to pump the Raylib music stream:

```cpp
audioManager.update(dt);  // called by GameEngine each frame
```

## Configuration

`AudioConfig` (in `RaylibAudioManager.hpp`) can be changed at construction time:

```cpp
AudioConfig cfg;
cfg.sounds[static_cast<size_t>(audio::SoundKind::WALK)].gain = 1.5f;
cfg.maxDist       = 50.f;
cfg.maxConcurrent = 12;
cfg.musicPath     = "assets/sound/custom_music.ogg";

auto audio = std::make_unique<RaylibAudioManager>(cfg);
```

## Adding a new sound effect

1. Add a value to `audio::SoundKind` in `include/audio/IAudioManager.hpp`.
2. Increment `AudioConfig::NUM_KINDS`.
3. Add a `SoundEntry` to `AudioConfig::sounds` in `RaylibAudioManager.hpp`.
4. Call `audioManager.playAt(SoundKind::MY_SOUND, pos)` from the relevant behavior or event handler.
