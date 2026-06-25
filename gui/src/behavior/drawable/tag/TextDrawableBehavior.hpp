#pragma once

#include "behavior/drawable/ADrawable.hpp"
#include "graphic/Types.hpp"

#include <string>

namespace behavior {

class TextDrawableBehavior : public ADrawable {
public:

    TextDrawableBehavior(const std::string& text, 
                         float size = 18.0f, 
                         graphic::Color4b color = {255, 255, 255, 255},
                         graphic::Anchor anchor = graphic::Anchor::Center)
        : _text(text), _size(size), _color(color), _isVisible(true), _anchor(anchor) {}


    void onUpdate([[maybe_unused]] graphic::Entity& owner, [[maybe_unused]] float deltaTime) override {}
    void draw(graphic::IRenderer& renderer, const graphic::Matrix4x4& transform) override;

    void setAnchor(graphic::Anchor anchor) { _anchor = anchor; }
    void setText(const std::string& text) { _text = text; }
    void setVisible(bool visible) { _isVisible = visible; } 

private:

    graphic::Vector2f getAnchoredPosition(const graphic::Vector2f& screenPos) const;

    std::string _text;
    float _size;
    graphic::Color4b _color;
    bool _isVisible;
    graphic::Anchor _anchor = graphic::Anchor::Center;
    
};

} // namespace behavior