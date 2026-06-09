#pragma once

#include "behavior/IBehavior.hpp"

namespace behavior {

class ABehavior : public IBehavior
{
    public:
        virtual ~ABehavior() = default;

        virtual void onUpdate(graphic::Entity& owner, float deltaTime) = 0;

        void onAttach(graphic::Entity&) override {}
        void onEvent(graphic::Entity&, const event::Event&) override {}
};

} // namespace behavior
