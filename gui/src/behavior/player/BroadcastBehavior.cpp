#include "BroadcastBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"

namespace behavior {

BroadcastBehavior::BroadcastBehavior(uint32_t playerId)
    : _playerId(playerId) {}

void BroadcastBehavior::onUpdate(graphic::Entity&, float dt)
{
    if (!_active) return;
    _timeLeft -= dt;
    if (_timeLeft <= 0.0f) {
        _active   = false;
        _timeLeft = 0.0f;
    }
}

void BroadcastBehavior::onEvent(graphic::Entity&, const event::Event& ev)
{
    event::on(ev,
        [&](const event::PlayerBroadcastEvent& e) {
            if (e.id != _playerId) return;
            _message  = e.message;
            _timeLeft = DISPLAY_DURATION;
            _active   = true;
        }
    );
}

} // namespace behavior
