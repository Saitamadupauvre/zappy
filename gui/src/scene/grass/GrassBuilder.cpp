#include "GrassBuilder.hpp"
#include "scene/TileShade.hpp"
#include <random>
#include <cmath>
#include <algorithm>

namespace zappy {

using graphic::Vector2f;
using graphic::Vector3f;
using graphic::Matrix4x4;
using graphic::Color4b;

namespace {

// Classic 2D Perlin gradient noise, returns roughly [-1, 1]. Self-contained (hashed
// gradients) so it needs no precomputed permutation table.
float fade(float t) { return t * t * t * (t * (t * 6.f - 15.f) + 10.f); }

float gradDot(int ix, int iy, float x, float y, unsigned seed)
{
    unsigned h = static_cast<unsigned>(ix) * 374761393u
               + static_cast<unsigned>(iy) * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    float angle = (h & 0xFFFFu) / 65535.f * 6.2831853f;
    return std::cos(angle) * (x - ix) + std::sin(angle) * (y - iy);
}

float perlin(float x, float y, unsigned seed)
{
    int x0 = static_cast<int>(std::floor(x)), y0 = static_cast<int>(std::floor(y));
    float u = fade(x - x0), v = fade(y - y0);
    float n00 = gradDot(x0,     y0,     x, y, seed);
    float n10 = gradDot(x0 + 1, y0,     x, y, seed);
    float n01 = gradDot(x0,     y0 + 1, x, y, seed);
    float n11 = gradDot(x0 + 1, y0 + 1, x, y, seed);
    float nx0 = n00 + u * (n10 - n00);
    float nx1 = n01 + u * (n11 - n01);
    return nx0 + v * (nx1 - nx0);
}

} // namespace

graphic::VertexData GrassBuilder::makeBladeMesh()
{
    graphic::VertexData d;
    // 2 stacked segments, tapering to a point at the tip.
    const int   segs    = 2;
    const float halfTop = 0.0f;
    const float halfBot = 0.5f; // half-width at the root (× per-instance width)

    for (int i = 0; i <= segs; ++i) {
        float h    = static_cast<float>(i) / segs;        // 0 root → 1 tip
        float half = halfBot + (halfTop - halfBot) * h;   // taper
        d.positions.push_back({-half, h, 0.f});
        d.positions.push_back({ half, h, 0.f});
        d.normals.push_back({0.f, 0.f, 1.f});
        d.normals.push_back({0.f, 0.f, 1.f});
        d.texCoords.push_back({0.f, h});
        d.texCoords.push_back({1.f, h});
    }
    for (int i = 0; i < segs; ++i) {
        unsigned short a = static_cast<unsigned short>(i * 2);
        unsigned short b = static_cast<unsigned short>(i * 2 + 1);
        unsigned short c = static_cast<unsigned short>(i * 2 + 2);
        unsigned short dd = static_cast<unsigned short>(i * 2 + 3);
        // Front face.
        d.indices.push_back(a); d.indices.push_back(c); d.indices.push_back(b);
        d.indices.push_back(b); d.indices.push_back(c); d.indices.push_back(dd);
        // Back face (reversed winding) so the blade is visible from both sides.
        d.indices.push_back(a); d.indices.push_back(b); d.indices.push_back(c);
        d.indices.push_back(b); d.indices.push_back(dd); d.indices.push_back(c);
    }
    return d;
}

GrassField GrassBuilder::build(const graphic::VertexData& surface,
                               int worldW, int worldH,
                               const GrassParams& params)
{
    GrassField field;
    field.blade = makeBladeMesh();

    const auto& pos = surface.positions;
    const auto& nrm = surface.normals;
    const auto& uv  = surface.texCoords;
    const auto& idx = surface.indices;
    if (idx.size() < 3) return field;

    const int triCount = static_cast<int>(idx.size() / 3);

    // Per-triangle area to distribute blades by surface area (uniform density).
    std::vector<float> triArea(triCount);
    float totalArea = 0.f;
    for (int t = 0; t < triCount; ++t) {
        const Vector3f& a = pos[idx[t*3]];
        const Vector3f& b = pos[idx[t*3+1]];
        const Vector3f& c = pos[idx[t*3+2]];
        float area = (b - a).cross(c - a).length() * 0.5f;
        triArea[t] = area;
        totalArea += area;
    }
    if (totalArea <= 0.f) return field;

    int blades = std::min(params.maxBlades,
                          static_cast<int>(params.density * totalArea));
    if (blades <= 0) return field;

    std::mt19937 rng(params.seed);
    std::uniform_real_distribution<float> u01(0.f, 1.f);
    std::discrete_distribution<int> pickTri(triArea.begin(), triArea.end());

    field.transforms.reserve(blades);
    field.colors.reserve(blades);

    auto lerp3 = [](const Vector3f& a, const Vector3f& b, const Vector3f& c,
                    float wa, float wb, float wc) {
        return Vector3f{a.x*wa + b.x*wb + c.x*wc,
                        a.y*wa + b.y*wb + c.y*wc,
                        a.z*wa + b.z*wb + c.z*wc};
    };
    auto lerp2 = [](const Vector2f& a, const Vector2f& b, const Vector2f& c,
                    float wa, float wb, float wc) {
        return Vector2f{a.x*wa + b.x*wb + c.x*wc, a.y*wa + b.y*wb + c.y*wc};
    };

    for (int i = 0; i < blades; ++i) {
        int t = pickTri(rng);
        unsigned i0 = idx[t*3], i1 = idx[t*3+1], i2 = idx[t*3+2];

        // Uniform barycentric sample inside the triangle.
        float r1 = u01(rng), r2 = u01(rng);
        if (r1 + r2 > 1.f) { r1 = 1.f - r1; r2 = 1.f - r2; }
        float wa = 1.f - r1 - r2, wb = r1, wc = r2;

        Vector3f p = lerp3(pos[i0], pos[i1], pos[i2], wa, wb, wc);
        Vector3f n = nrm.empty()
            ? Vector3f{0.f, 1.f, 0.f}
            : lerp3(nrm[i0], nrm[i1], nrm[i2], wa, wb, wc).normalized();

        float darkness = 1.f;
        if (params.tileShading && !uv.empty()) {
            Vector2f st = lerp2(uv[i0], uv[i1], uv[i2], wa, wb, wc);
            int tx = std::clamp(static_cast<int>(st.x * worldW), 0, std::max(0, worldW - 1));
            int ty = std::clamp(static_cast<int>(st.y * worldH), 0, std::max(0, worldH - 1));
            darkness = tileDarkness(tx, ty);
        }

        float height = params.minHeight + (params.maxHeight - params.minHeight) * u01(rng);
        float yaw    = u01(rng) * 2.f * static_cast<float>(M_PI);

        Matrix4x4 trans = Matrix4x4::Translation(p.x, p.y, p.z);
        Matrix4x4 rot   = Matrix4x4::RotationAlignUp(n, yaw);
        Matrix4x4 scale = Matrix4x4::Scale(params.width, height, params.width);
        // Row-vector convention: scale → rotate → translate (matches TransformBehavior).
        field.transforms.push_back(scale * rot * trans);

        // Smooth spatial colour patches: Perlin noise over the blade's world XZ,
        // mapped to [-1,1], on top of the tile darkness and per-blade random jitter.
        float noise = perlin(p.x * params.noiseScale, p.z * params.noiseScale, params.seed);

        // Brightness: tile darkness × random jitter × noise swing.
        float v = darkness * (0.85f + 0.3f * u01(rng)) * (1.f + params.noiseStrength * noise);
        // Green push so patches read as warmer/cooler grass, not just lighter/darker.
        float g = v * (1.f + params.noiseHueShift * noise);
        auto ch = [](unsigned char c, float m) {
            return static_cast<unsigned char>(std::clamp(c * m, 0.f, 255.f));
        };
        field.colors.push_back({ch(params.baseColor.r, v), ch(params.baseColor.g, g),
                                ch(params.baseColor.b, v), 255});
    }

    return field;
}

} // namespace zappy
