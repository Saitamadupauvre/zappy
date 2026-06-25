#pragma once
#include "behavior/ABehavior.hpp"
#include <functional>

namespace behavior {

class HoverableBehavior : public ABehavior {
public:
    using HoverCallback = std::function<void(graphic::Entity&)>;

    HoverableBehavior(HoverCallback onEnter, HoverCallback onLeave) 
        : _onEnter(onEnter), _onLeave(onLeave), _isHovered(false) {}

    void onUpdate([[maybe_unused]] graphic::Entity& owner, [[maybe_unused]] float deltaTime) override {}

    void onEvent(graphic::Entity& owner, const event::Event& event) override;

    bool isHovered() const { return _isHovered; }

    void setHovered(bool hovered, graphic::Entity& owner);

private:
    HoverCallback _onEnter;
    HoverCallback _onLeave;
    bool _isHovered;
};

} // namespace behavior