#pragma once

#include "scene/layout/IMapLayout.hpp"
#include "graphic/Types.hpp"
#include <cmath>
#include <vector>

namespace zappy {

// Maps the world grid onto a full torus surface (toroidal topology: both axes wrap).
// x → major angle θ ∈ [0, 2π] (wraps around the ring).
// y → minor angle φ ∈ [0, 2π] (wraps around the tube).
// Tiles are sampled at cell centres so they sit in the middle of each surface patch.
class TorusLayout : public IMapLayout
{
public:
    TorusLayout(float majorRadius = 8.0f, float minorRadius = 3.0f)
        : _R(majorRadius), _r(minorRadius) {}

    static float thetaAt(int x, int worldW)
    {
        return ((static_cast<float>(x) + 0.5f) / worldW) * 2.0f * M_PI;
    }
    static float phiAt(int y, int worldH)
    {
        return ((static_cast<float>(y) + 0.5f) / worldH) * 2.0f * M_PI;
    }

    graphic::Vector3f tilePos(int x, int y, int worldW, int worldH) const override
    {
        float theta = thetaAt(x, worldW);
        float phi   = phiAt(y, worldH);
        float cx = (_R + _r * std::cos(phi)) * std::cos(theta);
        float cy = _r * std::sin(phi);
        float cz = (_R + _r * std::cos(phi)) * std::sin(theta);
        return {cx, cy, cz};
    }

    float standY(int x, int y, int w, int h) const override
    {
        (void)x; (void)y; (void)w; (void)h;
        return 0.0f; // entity base sits directly on the surface
    }

    graphic::Vector3f upAt(int x, int y, int worldW, int worldH) const override
    {
        float theta = thetaAt(x, worldW);
        float phi   = phiAt(y, worldH);
        float nx = std::cos(phi) * std::cos(theta);
        float ny = std::sin(phi);
        float nz = std::cos(phi) * std::sin(theta);
        return {nx, ny, nz};
    }

    graphic::Vector3f forwardAt(int x, int y, int worldW, int worldH) const override
    {
        // Tangent along the tube (d pos / d phi): continuous over the surface.
        float theta = thetaAt(x, worldW);
        float phi   = phiAt(y, worldH);
        // North = decreasing y → -(d pos / d phi), matching the grid convention.
        float fx =  std::sin(phi) * std::cos(theta);
        float fy = -std::cos(phi);
        float fz =  std::sin(phi) * std::sin(theta);
        return graphic::Vector3f{fx, fy, fz}.normalized();
    }

    float majorRadius() const { return _R; }
    float minorRadius() const { return _r; }
    void  setMajorRadius(float R) { _R = R; }
    void  setMinorRadius(float r) { _r = r; }

    CameraFraming cameraFraming(int, int) const override
    {
        return {(_R + _r) * 2.5f, 35.0f};
    }

    bool animatesWrap() const override { return true; }

    // Ring/tube radii scaled to the world size. R: ring circumference fits worldW
    // tiles at SPACING (never below 6). r: tube radius for worldH tiles, kept under
    // 0.9*R so the torus keeps a hole. SPACING mirrors WorldScene::SPACING.
    void updateSizing(int worldW, int worldH) override
    {
        constexpr float SPACING = 2.0f;
        float R = std::max(6.0f, worldW * SPACING / (2.0f * static_cast<float>(M_PI)));
        float r = std::max(2.0f, worldH * SPACING / (2.0f * static_cast<float>(M_PI)));
        if (r > R * 0.45f) r = R * 0.45f; // cap tube radius low → big central hole
        _R = R;
        _r = r;
    }

    graphic::VertexData buildMesh(int worldW, int worldH) const override
    {
        graphic::VertexData torusData;
        const float tau = 2.0f * static_cast<float>(M_PI);

        for (int y = 0; y < worldH; ++y) {
            for (int x = 0; x < worldW; ++x) {
                float t0 = (static_cast<float>(x)     / worldW) * tau;
                float t1 = (static_cast<float>(x + 1) / worldW) * tau;
                float p0 = (static_cast<float>(y)     / worldH) * tau;
                float p1 = (static_cast<float>(y + 1) / worldH) * tau;
                auto patch = buildTorusPatch(t0, t1, p0, p1, 4, 4);

                // Remap local UVs [0,1]²  →  global tile UV so one texel = one tile.
                for (auto& tc : patch.texCoords) {
                    tc.x = (static_cast<float>(x) + tc.x) / static_cast<float>(worldW);
                    tc.y = (static_cast<float>(y) + tc.y) / static_cast<float>(worldH);
                }

                unsigned short baseIdx = static_cast<unsigned short>(torusData.positions.size());
                for (const auto& pos : patch.positions)
                    torusData.positions.push_back(pos);
                for (const auto& norm : patch.normals)
                    torusData.normals.push_back(norm);
                for (const auto& tc : patch.texCoords)
                    torusData.texCoords.push_back(tc);
                for (auto idx : patch.indices)
                    torusData.indices.push_back(idx + baseIdx);
            }
        }
        return torusData;
    }

private:
    float _R;
    float _r;

    graphic::VertexData buildTorusPatch(float theta0, float theta1, float phi0, float phi1,
                                        int segU, int segV) const
    {
        graphic::VertexData data;
        const int stride = segV + 1;

        for (int i = 0; i <= segU; ++i) {
            float theta = theta0 + (theta1 - theta0) * (static_cast<float>(i) / segU);
            float cosT = std::cos(theta), sinT = std::sin(theta);
            for (int j = 0; j <= segV; ++j) {
                float phi  = phi0 + (phi1 - phi0) * (static_cast<float>(j) / segV);
                float cosP = std::cos(phi), sinP = std::sin(phi);
                data.positions.push_back({(_R + _r * cosP) * cosT, _r * sinP, (_R + _r * cosP) * sinT});
                data.normals.push_back({cosP * cosT, sinP, cosP * sinT});
                data.texCoords.push_back({static_cast<float>(i) / segU,
                                          static_cast<float>(j) / segV});
            }
        }

        for (int i = 0; i < segU; ++i) {
            for (int j = 0; j < segV; ++j) {
                unsigned short a = static_cast<unsigned short>(i * stride + j);
                unsigned short b = static_cast<unsigned short>((i + 1) * stride + j);
                unsigned short c = static_cast<unsigned short>((i + 1) * stride + j + 1);
                unsigned short d = static_cast<unsigned short>(i * stride + j + 1);
                data.indices.push_back(a); data.indices.push_back(d); data.indices.push_back(c);
                data.indices.push_back(a); data.indices.push_back(c); data.indices.push_back(b);
            }
        }
        return data;
    }
};

} // namespace zappy
