#include "GroundDrawableBehavior.hpp"
#include "scene/grass/GrassBuilder.hpp"
#include "scene/TileShade.hpp"

namespace behavior {


GroundDrawableBehavior::GroundDrawableBehavior(graphic::IRenderer& renderer,
                                                    const graphic::VertexData& meshData,
                                                    int worldW, int worldH, bool showTiles)
    : _renderer(renderer)
{
    _mesh    = renderer.uploadMesh(meshData);
    _texture = renderer.uploadTexture(makeGroundTexture(worldW, worldH, showTiles));

    // Grow grass over the same surface mesh (works for any mesh — see GrassBuilder).
    zappy::GrassParams params;
    params.tileShading = showTiles;
    zappy::GrassField gf = zappy::GrassBuilder::build(meshData, worldW, worldH, params);
    _grass = renderer.uploadGrassField(gf.blade, gf.transforms, gf.colors);
}

GroundDrawableBehavior::~GroundDrawableBehavior()
{
    _renderer.unloadMesh(_mesh);
    _renderer.unloadTexture(_texture);
    _renderer.unloadGrassField(_grass);
}

void GroundDrawableBehavior::draw(graphic::IRenderer& renderer,
                                     const graphic::Matrix4x4& transform)
{
    renderer.drawMesh(graphic::MeshDrawParams{
        .mesh      = _mesh,
        .texture   = _texture,
        .transform = transform,
        .tint      = graphic::Color4b::white(),
    });

    if (_grassEnabled)
        renderer.drawGrassField(_grass, graphic::GrassDrawParams{
            .time         = _time,
            .windDir      = {1.f, 0.3f},
            .windStrength = 0.18f,
        });
}

// One pixel per world tile — green ground, darker where tileDarkness() darkens.
graphic::TextureData GroundDrawableBehavior::makeGroundTexture(int worldW, int worldH, bool showTiles)
{
    graphic::TextureData tex;
    tex.width    = worldW;
    tex.height   = worldH;
    tex.channels = 4;
    tex.pixels.resize(static_cast<size_t>(worldW * worldH * 4));

    constexpr unsigned char baseR = 70, baseG = 130, baseB = 55;
    for (int y = 0; y < worldH; ++y) {
        for (int x = 0; x < worldW; ++x) {
            float d = showTiles ? zappy::tileDarkness(x, y) : 1.0f;
            int i = (y * worldW + x) * 4;
            tex.pixels[i + 0] = static_cast<unsigned char>(baseR * d);
            tex.pixels[i + 1] = static_cast<unsigned char>(baseG * d);
            tex.pixels[i + 2] = static_cast<unsigned char>(baseB * d);
            tex.pixels[i + 3] = 255;
        }
    }
    return tex;
}

} // namespace behavior
