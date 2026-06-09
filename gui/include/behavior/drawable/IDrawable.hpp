#pragma once

#include "graphic/IRenderer.hpp"
#include "graphic/Types.hpp"

namespace behavior {

class IDrawable {
    public:
        virtual ~IDrawable() = default;

        virtual void draw(graphic::IRenderer& renderer,
                          const graphic::Matrix4x4& transform) = 0;
};

} // namespace behavior
