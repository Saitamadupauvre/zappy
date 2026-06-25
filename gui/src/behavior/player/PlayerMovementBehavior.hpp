#pragma once

#include "behavior/ABehavior.hpp"
#include <cstdint>

namespace behavior {

class PlayerMovementBehavior : public ABehavior
{
public:
    explicit PlayerMovementBehavior(uint32_t entityId);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

private:
    void applyMove(graphic::Entity& owner, const graphic::Vector3f& target, float duration);

    uint32_t          _entityId;
    graphic::Vector3f _startPos{0, 0, 0};
    graphic::Vector3f _endPos  {0, 0, 0};
    float             _elapsed  = 0.0f;
    float             _duration = 0.0f;
    bool              _moving   = false;
};

} // namespace behavior
