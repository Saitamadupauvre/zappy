#pragma once

#include "behavior/ABehavior.hpp"
#include "graphic/Vectors.hpp"
#include <cstdint>

namespace behavior {

class RotationBehavior : public ABehavior {
public:
    explicit RotationBehavior(uint32_t entityId);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

    const graphic::Vector3f& getTargetFwd() const { return _targetFwd; }
    const graphic::Vector3f& getTargetUp()  const { return _targetUp;  }
    float                    getTargetYaw() const { return _targetYaw; }

private:
    uint32_t          _entityId;
    float             _startYaw  = 0.0f;
    float             _targetYaw = 0.0f;
    float             _elapsed   = 0.0f;
    float             _duration  = 0.0f;
    bool              _rotating  = false;
    graphic::Vector3f _startUp   = graphic::Vector3f::up();
    graphic::Vector3f _targetUp  = graphic::Vector3f::up();
    graphic::Vector3f _startFwd  = graphic::Vector3f::forward();
    graphic::Vector3f _targetFwd = graphic::Vector3f::forward();
};

} // namespace behavior
