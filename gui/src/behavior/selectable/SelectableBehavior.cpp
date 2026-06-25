#include "SelectableBehavior.hpp"
#include "event/Event.hpp"
#include "locator/Locator.hpp"

namespace behavior {

void SelectableBehavior::setSelected(graphic::Entity& owner, bool selected) {
    if (_selected != selected) {
        _selected = selected;
        if (_onSelect) _onSelect(owner, _selected);
    }
}

void SelectableBehavior::toggle(graphic::Entity& owner) {
    setSelected(owner, !_selected);
}

void SelectableBehavior::onEvent(graphic::Entity& owner, const event::Event& ev) {
    event::on(ev, [&](const event::ClickEvent& e) {
        if (e.entityId == owner.getID()) {
            auto scene = zappy::Locator::getScene();
            if (scene) {
                scene->handleEvent(event::EntitySelectedEvent{owner.getID()});
            }
        }
    });
}

} // namespace behavior