#pragma once

#include "behavior/ABehavior.hpp"
#include "graphic/Types.hpp"
#include <cstdint>

namespace behavior {

class MovementBehavior : public ABehavior {
public:
    explicit MovementBehavior(uint32_t entityId);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

    bool isMoving() const { return _moving; }

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
