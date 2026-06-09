#pragma once

#include "behavior/ABehavior.hpp"
#include "behavior/drawable/IDrawable.hpp"

namespace behavior {

class ADrawable : public ABehavior, public IDrawable
{
    public:
        void onEvent(graphic::Entity& owner, const event::Event& event) override;
};

} // namespace behavior
