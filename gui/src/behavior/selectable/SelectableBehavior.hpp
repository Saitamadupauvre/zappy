#pragma once
#include "behavior/ABehavior.hpp"
#include <functional>
#include "entity/Entity.hpp"

namespace behavior {

class SelectableBehavior : public ABehavior {
public:
    using SelectCallback = std::function<void(graphic::Entity&, bool)>;

    SelectableBehavior(SelectCallback callback = [](graphic::Entity&, bool){}) 
        : _selected(false), _onSelect(callback) {}

    void setSelected(graphic::Entity& owner, bool selected);
    void toggle(graphic::Entity& owner);
    bool isSelected() const { return _selected; }

    void onUpdate([[maybe_unused]] graphic::Entity& owner, [[maybe_unused]] float deltaTime) override {}
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

private:
    bool _selected;
    SelectCallback _onSelect;
};

} // namespace behavior