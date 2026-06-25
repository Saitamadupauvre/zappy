#pragma once

#include "behavior/drawable/ADrawable.hpp"
#include "graphic/Types.hpp"

namespace behavior {

class MeshDrawableBehavior : public ADrawable
{
    public:
        MeshDrawableBehavior(graphic::MeshHandle mesh, graphic::TextureHandle texture,
                             graphic::Color4b tint = graphic::Color4b::white());

        void setTint(graphic::Color4b tint) { _tint = tint; }
        void setVisible(bool visible)       { _visible = visible; }

        void onUpdate(graphic::Entity&, float) override {}
        void draw(graphic::IRenderer& renderer, const graphic::Matrix4x4& transform) override;

        graphic::MeshHandle getMesh() const override { return _mesh; }
        bool isVisible() const override { return _visible; }

    private:
        graphic::MeshHandle    _mesh;
        graphic::TextureHandle _texture;
        graphic::Color4b       _tint    = graphic::Color4b::white();
        bool                   _visible = true;
};

} // namespace behavior
