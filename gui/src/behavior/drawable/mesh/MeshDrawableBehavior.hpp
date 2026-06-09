#pragma once

#include "behavior/drawable/ADrawable.hpp"
#include "graphic/Types.hpp"

namespace behavior {

class MeshDrawableBehavior : public ADrawable
{
    public:
        MeshDrawableBehavior(graphic::MeshHandle mesh, graphic::TextureHandle texture);

        void onUpdate(graphic::Entity&, float) override {}
        void draw(graphic::IRenderer& renderer, const graphic::Matrix4x4& transform) override;

    private:
        graphic::MeshHandle _mesh;
        graphic::TextureHandle _texture;
};

} // namespace behavior
