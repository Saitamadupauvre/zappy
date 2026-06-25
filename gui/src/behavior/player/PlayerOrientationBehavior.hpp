#pragma once

#include "behavior/ABehavior.hpp"
#include <cstdint>

namespace behavior {

class PlayerOrientationBehavior : public ABehavior
{
public:
    explicit PlayerOrientationBehavior(uint32_t entityId);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

private:
    uint32_t _entityId;
    float    _startYaw  = 0.0f;
    float    _targetYaw = 0.0f;
    float    _elapsed   = 0.0f;
    float    _duration  = 0.0f;
    bool     _rotating  = false;
};

} // namespace behavior
