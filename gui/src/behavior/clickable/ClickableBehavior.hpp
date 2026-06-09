#pragma once
#include "behavior/ABehavior.hpp"
#include <functional>

namespace behavior {

class ClickableBehavior : public ABehavior {
public:
    using ClickCallback = std::function<void(graphic::Entity&)>;

    ClickableBehavior(ClickCallback callback) : _onClick(callback) {}

    void onUpdate(graphic::Entity& owner, float deltaTime) override {}

    void setOnClick(ClickCallback callback) { _onClick = callback; }

    void triggerClick(graphic::Entity& owner) {
        if (_onClick) {
            _onClick(owner);
        }
    }

private:
    ClickCallback _onClick;
};

} // namespace behavior