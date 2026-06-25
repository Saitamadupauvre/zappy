#include "PlayerBehavior.hpp"
#include "event/Event.hpp"
#include "entity/Entity.hpp"

namespace behavior {

PlayerBehavior::PlayerBehavior(const zappy::PlayerState& state)
    : _playerId(state.id), _x(state.x), _y(state.y),
      _orientation(state.orientation), _level(state.level),
      _team(state.team), _inventory(state.inventory) {}

void PlayerBehavior::onEvent(graphic::Entity&, const event::Event& ev)
{
    event::on(ev,
        [&](const event::PlayerMovedEvent& e) {
            if (e.id != _playerId) return;
            _x = e.x;
            _y = e.y;
            _orientation = e.orientation;
        },
        [&](const event::PlayerLevelChangedEvent& e) {
            if (e.id != _playerId) return;
            _level = e.level;
        },
        [&](const event::PlayerInventoryChangedEvent& e) {
            if (e.id != _playerId) return;
            _inventory = e.inventory;
        }
    );
}

} // namespace behavior
