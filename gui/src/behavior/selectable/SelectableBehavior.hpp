#pragma once
#include "behavior/ABehavior.hpp"

namespace behavior {

class SelectableBehavior : public ABehavior {
public:
    SelectableBehavior() : _selected(false) {}
    ~SelectableBehavior() override = default;

    void setSelected(bool selected) { _selected = selected; }
    bool isSelected() const { return _selected; }

    void toggle() { _selected = !_selected; }

    void onUpdate(graphic::Entity& owner, float deltaTime) override {}

private:
    bool _selected;
};

} // namespace behavior