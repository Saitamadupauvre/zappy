#include "OutlineBehavior.hpp"
#include <algorithm>
#include "behavior/selectable/SelectableBehavior.hpp"
#include "behavior/hoverable/HoverableBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "behavior/drawable/ADrawable.hpp"
#include "entity/Entity.hpp"
#include <iostream>

namespace behavior {

void OutlineBehavior::onUpdate([[maybe_unused]] graphic::Entity& owner, [[maybe_unused]] float deltaTime) {
    updateOutlineState();
}

void OutlineBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::SelectEvent& e) {
            if (e.entityId != owner.getID()) return;
            _isSelected = e.isSelected;
            updateOutlineState();
        },
        [&](const event::HoverEvent& e) {
            if (e.entityId != owner.getID()) return;
            _isHovered = e.isHovered;
            updateOutlineState();
        },
        [&](const event::TeamSelectEvent& e) {
            auto it = std::find(e.ids.begin(), e.ids.end(), owner.getID());
            _isSelected = e.isSelected && (it != e.ids.end());
            updateOutlineState();
        }
    );

    event::on(ev, [&](const event::RenderEvent& e) {
        if (!_shouldOutline) return;
        
        auto drawable = owner.getBehavior<behavior::ADrawable>();
        if (!drawable || !drawable->isVisible()) return;

        auto t = owner.getBehavior<behavior::TransformBehavior>();
        auto matrix = t ? t->getMatrix() : graphic::Matrix4x4::Identity();

        drawOutline(owner, e.renderer, matrix);
    });
}

void OutlineBehavior::updateOutlineState() {
    _shouldOutline = _isSelected || _isHovered;
}

void OutlineBehavior::setHovered(bool hovered) {
        _isHovered = hovered;
        updateOutlineState();
}

void OutlineBehavior::setSelected(bool selected) {
    _isSelected = selected;
    updateOutlineState();
}

void OutlineBehavior::drawOutline(graphic::Entity& owner, graphic::IRenderer& renderer, const graphic::Matrix4x4& transform) {
    if (!_shouldOutline) return;

    auto drawable = owner.getBehavior<behavior::ADrawable>();
    if (!drawable || !drawable->isVisible()) return;

    graphic::Color4b color = _isSelected ? 
        graphic::Color4b{255, 255, 0, 255} : 
        graphic::Color4b{0, 255, 255, 255};

    renderer.drawOutline(graphic::MeshDrawParams{
        .mesh      = drawable->getMesh(),
        .texture   = {},
        .transform = transform,
        .tint      = color
    }, 0.05f, color);
}

} // namespace behavior