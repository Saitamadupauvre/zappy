#pragma once
#include "behavior/ABehavior.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/Types.hpp"

namespace behavior {

class OutlineBehavior : public ABehavior {
public:
    OutlineBehavior() = default;
    
    void drawOutline(graphic::Entity& owner, graphic::IRenderer& renderer, const graphic::Matrix4x4& transform);    
    void setHovered(bool hovered);
    void setSelected(bool selected);
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;
    void onUpdate(graphic::Entity& owner, float deltaTime) override;

private:
    void updateOutlineState();
    bool _isSelected = false;
    bool _isHovered = false;
    bool _shouldOutline = false;
};

} // namespace behavior