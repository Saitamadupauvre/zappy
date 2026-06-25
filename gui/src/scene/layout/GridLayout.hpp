#pragma once

#include "scene/layout/IMapLayout.hpp"
#include <algorithm>

namespace zappy {

class GridLayout : public IMapLayout
{
public:
    explicit GridLayout(float spacing = 2.0f) : _sp(spacing) {}

    graphic::Vector3f tilePos(int x, int y, int worldW, int worldH) const override
    {
        float ox = (worldW - 1) * 0.5f * _sp;
        float oz = (worldH - 1) * 0.5f * _sp;
        return {x * _sp - ox, 0.0f, y * _sp - oz};
    }

    float standY(int, int, int, int) const override { return 0.0f; }

    graphic::Vector3f upAt(int, int, int, int) const override
    {
        return graphic::Vector3f::up();
    }

    graphic::Vector3f forwardAt(int, int, int, int) const override
    {
        return {0.0f, 0.0f, -1.0f}; // north = -Z (decreasing row)
    }

    float spacing() const { return _sp; }

    // A very thin box spanning the whole grid, top face at y=0 so entities and
    // resources rest on it. Top-face UVs span [0,1]² across the grid (one texel =
    // one tile), matching the ground texture / grass darkness lookup. The mesh is
    // world-space because the grass behavior bakes blade transforms from raw verts.
    graphic::VertexData buildMesh(int worldW, int worldH) const override
    {
        const float HX = worldW * _sp * 0.5f;
        const float HZ = worldH * _sp * 0.5f;
        const float T  = 0.2f; // thickness; bottom at -T, top at 0

        graphic::VertexData m;
        auto face = [&](graphic::Vector3f a, graphic::Vector3f b,
                        graphic::Vector3f c, graphic::Vector3f d,
                        graphic::Vector3f n,
                        graphic::Vector2f ua, graphic::Vector2f ub,
                        graphic::Vector2f uc, graphic::Vector2f ud) {
            unsigned short base = static_cast<unsigned short>(m.positions.size());
            for (auto& p : {a, b, c, d}) m.positions.push_back(p);
            for (int i = 0; i < 4; ++i) m.normals.push_back(n);
            for (auto& uv : {ua, ub, uc, ud}) m.texCoords.push_back(uv);
            for (auto i : {0, 1, 2, 0, 2, 3})
                m.indices.push_back(static_cast<unsigned short>(base + i));
        };
        graphic::Vector2f u00{0, 0}, u10{1, 0}, u11{1, 1}, u01{0, 1};

        // Top (visible playfield) — full grid UVs, grass grows here.
        face({-HX, 0, -HZ}, {-HX, 0, HZ}, {HX, 0, HZ}, {HX, 0, -HZ},
             {0, 1, 0}, u01, u00, u10, u11);
        // Bottom.
        face({-HX, -T, -HZ}, {HX, -T, -HZ}, {HX, -T, HZ}, {-HX, -T, HZ},
             {0, -1, 0}, u00, u10, u11, u01);
        // Thin sides — collapsed to tile (0,0) for the darkness lookup.
        face({-HX, -T, HZ}, {HX, -T, HZ}, {HX, 0, HZ}, {-HX, 0, HZ}, {0, 0, 1}, u00, u00, u00, u00);
        face({HX, -T, -HZ}, {-HX, -T, -HZ}, {-HX, 0, -HZ}, {HX, 0, -HZ}, {0, 0, -1}, u00, u00, u00, u00);
        face({-HX, -T, -HZ}, {-HX, -T, HZ}, {-HX, 0, HZ}, {-HX, 0, -HZ}, {-1, 0, 0}, u00, u00, u00, u00);
        face({HX, -T, HZ}, {HX, -T, -HZ}, {HX, 0, -HZ}, {HX, 0, HZ}, {1, 0, 0}, u00, u00, u00, u00);
        return m;
    }

    CameraFraming cameraFraming(int worldW, int worldH) const override
    {
        return {std::max(worldW, worldH) * _sp * 0.8f, 45.0f};
    }

    bool animatesWrap() const override { return false; }

private:
    float _sp;
};

} // namespace zappy
