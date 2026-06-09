#include "MeshDrawableBehavior.hpp"

namespace behavior {

MeshDrawableBehavior::MeshDrawableBehavior(graphic::MeshHandle mesh,
                                             graphic::TextureHandle texture)
    : _mesh(mesh), _texture(texture) {}

void MeshDrawableBehavior::draw(graphic::IRenderer& renderer,
                                 const graphic::Matrix4x4& transform) {
    renderer.drawMesh(graphic::MeshDrawParams{
        .mesh      = _mesh,
        .texture   = _texture,
        .transform = transform,
    });
}

} // namespace behavior
