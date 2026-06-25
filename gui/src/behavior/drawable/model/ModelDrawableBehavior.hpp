#pragma once

#include "behavior/drawable/ADrawable.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/Types.hpp"
#include "graphic/Matrix4x4.hpp"
#include <vector>
#include <string>

namespace behavior {

// Draws an animated model loaded from file. When ownsModel is true, unloads the
// model from the renderer on destruction (each player loads its own). When false,
// the model handle is shared (e.g. one crystal model reused across every tile's
// resources) and its lifetime is managed by the owning factory.
class ModelDrawableBehavior : public ADrawable
{
    public:
        ModelDrawableBehavior(graphic::IRenderer& renderer, graphic::ModelHandle model,
                              graphic::Color4b tint = graphic::Color4b::white(),
                              bool ownsModel = true);
        ~ModelDrawableBehavior() override;

        void setTint(graphic::Color4b tint)  { _tint = tint; }
        void setVisible(bool visible)        { _visible = visible; }
        void setModel(graphic::ModelHandle model) { _model = model; _mesh = _renderer->meshFromModel(model); }
        void setMeshTints(std::vector<graphic::Color4b> tints)    { _meshTints   = std::move(tints); }
        void setMeshShaders(std::vector<std::string> shaders)     { _meshShaders = std::move(shaders); }

        // Fixed local rotation (Euler degrees) baked before the entity transform.
        // Use to correct models exported facing the wrong axis.
        void setRotationOffset(const graphic::Vector3f& eulerDeg);

        void onUpdate(graphic::Entity&, float) override {}
        void draw(graphic::IRenderer& renderer, const graphic::Matrix4x4& transform) override;

        graphic::ModelHandle getModel() const { return _model; }
        graphic::MeshHandle  getMesh() const override { return _mesh; }
        bool                 isVisible() const override { return _visible; }

    private:
        graphic::IRenderer*  _renderer;
        graphic::ModelHandle _model;
        graphic::MeshHandle  _mesh;
        graphic::Color4b     _tint    = graphic::Color4b::white();
        bool                       _ownsModel  = true;
        bool                       _visible    = true;
        std::vector<graphic::Color4b> _meshTints;
        std::vector<std::string>      _meshShaders;
        graphic::Matrix4x4   _offset  = graphic::Matrix4x4::Identity();
        bool                 _hasOffset = false;
};

} // namespace behavior
