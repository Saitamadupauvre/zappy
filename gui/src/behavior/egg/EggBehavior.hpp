#pragma once

#include "behavior/ABehavior.hpp"
#include <cstdint>

namespace behavior {

// Pulsates the egg's scale. Shrinks and self-signals removal on hatch/die.
class EggBehavior : public ABehavior
{
public:
    explicit EggBehavior(uint32_t eggId);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

    bool shouldRemove() const { return _pendingRemoval && _removeTimer <= 0.0f; }

private:
    uint32_t _eggId;
    float    _time           = 0.0f;
    bool     _pendingRemoval = false;
    float    _removeTimer    = 0.0f;
};

} // namespace behavior
