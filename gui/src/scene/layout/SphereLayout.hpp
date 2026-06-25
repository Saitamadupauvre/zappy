#pragma once

#include "scene/layout/IMapLayout.hpp"
#include <cmath>

namespace zappy {

// Maps the world grid onto a sphere surface.
// x → longitude (azimuth), y → latitude (polar angle).
class SphereLayout : public IMapLayout
{
public:
    explicit SphereLayout(float radius = 8.0f) : _r(radius) {}

    graphic::Vector3f tilePos(int x, int y, int worldW, int worldH) const override
    {
        float theta = (static_cast<float>(x) / worldW) * 2.0f * M_PI;
        float phi   = (static_cast<float>(y) / worldH) * M_PI;
        return {
            _r * std::sin(phi) * std::cos(theta),
            _r * std::cos(phi),
            _r * std::sin(phi) * std::sin(theta)
        };
    }

    float standY(int x, int y, int w, int h) const override
    {
        (void)x; (void)y; (void)w; (void)h;
        return 0.2f;
    }

    graphic::Vector3f upAt(int x, int y, int worldW, int worldH) const override
    {
        auto p = tilePos(x, y, worldW, worldH);
        float len = std::sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
        if (len < 1e-6f) return graphic::Vector3f::up();
        return {p.x/len, p.y/len, p.z/len};
    }

    graphic::Vector3f forwardAt(int x, int y, int worldW, int worldH) const override
    {
        // Tangent along latitude (d pos / d phi): continuous over the sphere.
        float theta = (static_cast<float>(x) / worldW) * 2.0f * M_PI;
        float phi   = (static_cast<float>(y) / worldH) * M_PI;
        float fx =  std::cos(phi) * std::cos(theta);
        float fy = -std::sin(phi);
        float fz =  std::cos(phi) * std::sin(theta);
        return graphic::Vector3f{fx, fy, fz}.normalized();
    }

private:
    float _r;
};

} // namespace zappy
