#pragma once

#include "graphic/Vectors.hpp"

namespace audio {

enum class SoundKind {
    WALK,
    PICKUP,
    INCANTATION,
    EXPLOSION,
    BROADCAST,
};

class IAudioManager {
public:
    virtual ~IAudioManager() = default;
    virtual void init()                                            = 0;
    virtual void update(float dt)                                  = 0;
    virtual void playAt(SoundKind kind, graphic::Vector3f worldPos)= 0;
    virtual void stopSound(SoundKind kind)                         = 0;
    virtual void setListenerPos(graphic::Vector3f pos)             = 0;
    virtual void setSoundVolume(float vol)                         = 0;
    virtual void setMusicVolume(float vol)                         = 0;
    virtual void destroy()                                         = 0;
};

} // namespace audio
