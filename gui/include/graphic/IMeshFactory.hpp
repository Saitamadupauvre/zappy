#pragma once

#include "graphic/Types.hpp"

namespace graphic {

class IMeshFactory {
public:
    virtual ~IMeshFactory() = default;

    virtual VertexData createCube    (float width, float height, float length) = 0;
    virtual VertexData createSphere  (float radius, int rings, int slices) = 0;
    virtual VertexData createTorus   (float radius, float tubeRadius, int radialSegments, int tubularSegments) = 0;
    virtual VertexData createPlane   (float width, float height) = 0;
    virtual VertexData createCylinder(float radius, float height, int slices) = 0;
};

} // namespace graphic
