#pragma once
#include "behavior/ABehavior.hpp"
#include "entity/Entity.hpp"
#include <functional>

namespace behavior {

class ClickableBehavior : public ABehavior {
public:
    using ClickCallback = std::function<void(graphic::Entity&)>;

    ClickableBehavior(ClickCallback callback = [](graphic::Entity&){}) 
        : _onClick(callback) {}

    void onUpdate(graphic::Entity& owner, float deltaTime) override {
        (void)owner;
        (void)deltaTime;
    }

    void setOnClick(ClickCallback callback) { _onClick = callback; }

    void onEvent(graphic::Entity& owner, const event::Event& ev) override {
        event::on(ev,
            [&](const event::ClickEvent& e) {
                if (e.entityId == owner.getID() && _onClick) _onClick(owner);
            }
        );
    }

private:
    ClickCallback _onClick;
};

} // namespace behavior