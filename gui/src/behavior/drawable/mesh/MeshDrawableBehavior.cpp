#include "MeshDrawableBehavior.hpp"

namespace behavior {

MeshDrawableBehavior::MeshDrawableBehavior(graphic::MeshHandle mesh,
                                             graphic::TextureHandle texture,
                                             graphic::Color4b tint)
    : _mesh(mesh), _texture(texture), _tint(tint) {}

void MeshDrawableBehavior::draw(graphic::IRenderer& renderer,
                                 const graphic::Matrix4x4& transform) {
    if (!_visible) return;
    renderer.drawMesh(graphic::MeshDrawParams{
        .mesh      = _mesh,
        .texture   = _texture,
        .transform = transform,
        .tint      = _tint,
    });
}

} // namespace behavior
