#pragma once

#include "event/Event.hpp"

namespace graphic { class Entity; }

namespace behavior {

class IBehavior
{
    public:
        virtual ~IBehavior() = default;

        virtual void onAttach(graphic::Entity& owner) = 0;
        virtual void onUpdate(graphic::Entity& owner, float deltaTime) = 0;
        virtual void onEvent(graphic::Entity& owner, const event::Event& event) = 0;
};

} // namespace behavior
