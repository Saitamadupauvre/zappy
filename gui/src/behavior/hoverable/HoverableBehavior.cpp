#include "HoverableBehavior.hpp"

namespace behavior {

void HoverableBehavior::setHovered(bool hovered, graphic::Entity& owner) {
    if (hovered != _isHovered) {
        _isHovered = hovered;
        
        if (_isHovered && _onEnter) {
            _onEnter(owner);
        } else if (!_isHovered && _onLeave) {
            _onLeave(owner);
        }
    }
}

} // namespace behavior