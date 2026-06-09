#include "OutlineBehavior.hpp"
#include "SelectableBehavior.hpp"
#include "HoverableBehavior.hpp"

namespace behavior {

void OutlineBehavior::onUpdate(graphic::Entity& owner, float deltaTime) {
    auto selectable = owner.getBehavior<SelectableBehavior>();
    auto hoverable = owner.getBehavior<HoverableBehavior>();

    if ((selectable && selectable->isSelected()) || (hoverable && hoverable->isHovered())) {
        renderOutline(owner);
    }
}

void OutlineBehavior::renderOutline(graphic::Entity& owner) {}

} // namespace behavior