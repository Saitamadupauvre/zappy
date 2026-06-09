#pragma once
#include "behavior/ABehavior.hpp"

namespace behavior {

class OutlineBehavior : public ABehavior {
public:
    OutlineBehavior() = default;
    ~OutlineBehavior() override = default;

    void onUpdate(graphic::Entity& owner, float deltaTime) override;

private:
    void renderOutline(graphic::Entity& owner);
};

} // namespace behavior