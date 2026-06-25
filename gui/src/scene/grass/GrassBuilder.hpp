#pragma once

#include "graphic/Types.hpp"
#include "graphic/Math.hpp"
#include <vector>

namespace zappy {

// Reusable, mesh-agnostic grass generator. Scatters blade instances over the
// triangles of ANY source surface (torus, cube, …), orienting each blade along
// the interpolated surface normal. Per-instance tint carries tile darkness so
// the grass darkens exactly where the ground does.
struct GrassField {
    graphic::VertexData             blade;       // shared per-blade geometry
    std::vector<graphic::Matrix4x4> transforms;  // one model matrix per blade
    std::vector<graphic::Color4b>   colors;       // one tint per blade
};

struct GrassParams {
    float           density   = 160.0f;   // blades per unit of surface area
    int             maxBlades = 350000;   // hard cap (keeps instancing bounded)
    float           minHeight = 0.16f;
    float           maxHeight = 0.34f;
    float           width     = 0.06f;
    // Matches the ground tile base colour (see GroundDrawableBehavior); the
    // per-tile darkness is multiplied in at build time so blades share the tile colour.
    graphic::Color4b baseColor = {70, 130, 55, 255};
    unsigned        seed       = 1337u;
    // Perlin noise adds smooth, spatially-coherent colour patches on top of the
    // per-tile darkness and the per-blade random jitter.
    float           noiseScale    = 0.50f; // patch frequency (world units → noise space)
    float           noiseStrength = 0.18f; // ± brightness swing from the noise
    float           noiseHueShift = 0.20f; // green-channel push for warm/cool patches
    // When false the per-tile checkerboard darkness is ignored and only the Perlin
    // noise drives colour variation (uniform base tint + smooth patches).
    bool            tileShading   = true;
};

class GrassBuilder {
public:
    // worldW/worldH map the surface UVs to tile coordinates for the darkness
    // lookup (see scene/TileShade.hpp). For meshes without a tile grid, pass 1,1.
    static GrassField build(const graphic::VertexData& surface,
                            int worldW, int worldH,
                            const GrassParams& params = {});

    // A small tapered, double-sided blade of unit height (local +Y up, root at
    // y=0). texCoord.y encodes root→tip; the transform scales it per instance.
    static graphic::VertexData makeBladeMesh();
};

} // namespace zappy
