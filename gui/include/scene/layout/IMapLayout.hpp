#pragma once

#include "graphic/Types.hpp"

namespace zappy {

// Orbit-camera framing a layout wants when it becomes active.
struct CameraFraming {
    float distance;
    float pitch;
};

class IMapLayout
{
public:
    virtual ~IMapLayout() = default;

    virtual graphic::Vector3f tilePos(int x, int y, int worldW, int worldH) const = 0;
    virtual float             standY (int x, int y, int worldW, int worldH) const = 0;
    virtual graphic::Vector3f upAt   (int x, int y, int worldW, int worldH) const = 0;
    // World-space "north" (server orientation 1) tangent at this tile. Must lie
    // in the tile's tangent plane and vary continuously across the surface.
    virtual graphic::Vector3f forwardAt(int x, int y, int worldW, int worldH) const = 0;

    // The ground surface mesh for a world of this size (world-space, fed to the
    // grass-covered ground behavior).
    virtual graphic::VertexData buildMesh(int worldW, int worldH) const = 0;
    // Camera framing for this layout at the given world size.
    virtual CameraFraming cameraFraming(int worldW, int worldH) const = 0;
    // True when edge-wrap steps are physically continuous (closed surface) and
    // should animate; false for a flat grid that must teleport across a wrap.
    virtual bool animatesWrap() const = 0;
    // Recompute any size-dependent internal state. No-op by default.
    virtual void updateSizing(int worldW, int worldH) { (void)worldW; (void)worldH; }
};

} // namespace zappy
