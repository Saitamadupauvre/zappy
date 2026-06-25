#pragma once

#include <functional>
#include "behavior/ABehavior.hpp"

namespace behavior {

class RectTransformBehavior : public ABehavior {
public:

    RectTransformBehavior() = default;

    void setPosition(const graphic::Vector2f& pos) { _position = pos; }
    void setSize(const graphic::Vector2f& size) { _size = size; }
    void onUpdate([[maybe_unused]] graphic::Entity& owner, [[maybe_unused]] float deltaTime) {};

    const graphic::Vector2f& getPosition() const { return _position; }
    const graphic::Vector2f& getSize() const { return _size; }

private:
    graphic::Vector2f _position{0.0f, 0.0f};
    graphic::Vector2f _size{100.0f, 100.0f};
};

}