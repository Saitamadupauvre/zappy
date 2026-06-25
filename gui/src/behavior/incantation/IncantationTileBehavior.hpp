#pragma once

#include "behavior/ABehavior.hpp"
#include "scene/world/AnimationClock.hpp"

namespace behavior {

// Attached to tile entities. Pulses the tile tint during incantation,
// flashes gold on success or red on failure, then restores the base tint.
class IncantationTileBehavior : public ABehavior
{
public:
    IncantationTileBehavior(int tileX, int tileY,
                            graphic::Color4b baseTint,
                            const zappy::AnimationClock& clock);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

private:
    int                          _tileX;
    int                          _tileY;
    graphic::Color4b             _baseTint;
    const zappy::AnimationClock& _clock;

    bool  _active      = false;
    float _time        = 0.0f;
    float _flashTimer  = 0.0f;
    bool  _flashing    = false;
    bool  _flashSuccess= false;
};

} // namespace behavior
