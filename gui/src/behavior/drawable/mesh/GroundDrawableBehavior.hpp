#pragma once

#include "behavior/drawable/ADrawable.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/Types.hpp"

namespace behavior {

// Draws a grass-covered ground surface from any mesh (flat grid box, torus, …):
// the textured ground mesh plus a wind-animated instanced grass field grown over it.
class GroundDrawableBehavior : public ADrawable
{
public:
    GroundDrawableBehavior(graphic::IRenderer& renderer,
                           const graphic::VertexData& meshData,
                           int worldW, int worldH, bool showTiles);
    ~GroundDrawableBehavior() override;

    void onUpdate(graphic::Entity&, float dt) override { _time += dt; }
    void draw(graphic::IRenderer& renderer,
              const graphic::Matrix4x4& transform) override;

    graphic::MeshHandle getMeshHandle() const { return _mesh; }
    void setGrassEnabled(bool v) { _grassEnabled = v; }

private:
    // Green ground tint, darker on the checkerboard tiles when showTiles is true;
    // a flat uniform green when false (Perlin on the grass carries the variation).
    static graphic::TextureData makeGroundTexture(int worldW, int worldH, bool showTiles);

    graphic::IRenderer&       _renderer;
    graphic::MeshHandle       _mesh{};
    graphic::TextureHandle    _texture{};
    graphic::GrassFieldHandle _grass{};
    float                     _time         = 0.f;
    bool                      _grassEnabled = true;
};

} // namespace behavior
