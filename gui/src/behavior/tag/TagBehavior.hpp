#pragma once

#include "behavior/ABehavior.hpp"
#include "entity/Entity.hpp"
#include <string>

namespace behavior {

class TagBehavior : public ABehavior {
public:
    TagBehavior(uint32_t tagEntityId, float offsetY = 1.5f);

    void onUpdate(graphic::Entity& owner, float deltaTime) override;
    void onEvent(graphic::Entity& owner, const event::Event& event) override;

private:
    uint32_t _tagEntityId;
    float    _offsetY;
    bool _isSelected;
};
}