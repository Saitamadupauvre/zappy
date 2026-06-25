#include "TextDrawableBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "entity/Entity.hpp"

namespace behavior {

graphic::Vector2f TextDrawableBehavior::getAnchoredPosition(const graphic::Vector2f& screenPos) const
{
    float width = _text.length() * (_size * 0.3f);
    float height = _size * 0.5f;

    switch (_anchor) {
        case graphic::Anchor::Center:
            return { screenPos.x - (width / 2.0f), screenPos.y - (height / 2.0f) };
            
        case graphic::Anchor::BottomCenter:
            return { screenPos.x - (width / 2.0f), screenPos.y };
            
        case graphic::Anchor::TopLeft:
        default:
            return screenPos;
    }
}

void TextDrawableBehavior::draw(graphic::IRenderer& renderer, const graphic::Matrix4x4& transform)
{
    if (!_isVisible) return;

    graphic::Vector3f pos3D = transform.getTranslation();
    graphic::Vector2f screenPos = renderer.worldToScreen(pos3D);

    graphic::Vector2f anchoredPos = getAnchoredPosition(screenPos);

    graphic::TextStyle style;
    style.size = _size;
    style.color = _color;

    renderer.drawText(_text, anchoredPos, style);
}

} // namespace behavior