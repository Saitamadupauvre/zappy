#pragma once

#include "behavior/ABehavior.hpp"
#include <cstdint>
#include <string>

namespace behavior {

class BroadcastBehavior : public ABehavior
{
public:
    explicit BroadcastBehavior(uint32_t playerId);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

    bool isBroadcasting() const { return _active; }

private:
    static constexpr float DISPLAY_DURATION = 3.0f;

    uint32_t    _playerId;
    std::string _message;
    float       _timeLeft = 0.0f;
    bool        _active   = false;
};

} // namespace behavior
