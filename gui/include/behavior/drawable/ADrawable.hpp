#pragma once

#include "behavior/ABehavior.hpp"
#include "behavior/drawable/IDrawable.hpp"
#include "graphic/Types.hpp"

namespace behavior {

class ADrawable : public ABehavior, public IDrawable
{
    public:
        void onEvent(graphic::Entity& owner, const event::Event& event) override;

        // Mesh handle backing this drawable, for mesh-based systems (picking,
        // outline). Drawables without a pickable mesh return an empty handle.
        virtual graphic::MeshHandle getMesh() const { return {}; }
        virtual bool isVisible() const { return true; }
};

} // namespace behavior
