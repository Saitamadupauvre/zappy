#include "HoverableBehavior.hpp"
#include "event/Event.hpp"
#include "entity/Entity.hpp"

namespace behavior {

void HoverableBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::HoverEvent& e) {
            if (e.entityId != owner.getID()) return;
            setHovered(e.isHovered, owner);
        }
    );
}

void HoverableBehavior::setHovered(bool hovered, graphic::Entity& owner) {
    if (hovered != _isHovered) {
        _isHovered = hovered;
        
        if (_isHovered) {
            if (_onEnter)
                _onEnter(owner);
        } else {
            if (_onLeave)
                _onLeave(owner);
        }
    }
}

} // namespace behavior