#pragma once

#include "graphic/IMeshFactory.hpp"

#include <raylib.h>

namespace graphic::raylib {

/**
 * @class RaylibMeshFactory
 * @brief Concrete factory implementing procedural mesh generation using Raylib.
 */
class RaylibMeshFactory : public IMeshFactory {
    public:
        RaylibMeshFactory() = default;
        ~RaylibMeshFactory() override = default;

        VertexData createCube    (float width, float height, float length) override;
        VertexData createSphere  (float radius, int rings, int slices) override;
        VertexData createTorus   (float radius, float tubeRadius, int radialSegments, int tubularSegments) override;
        VertexData createPlane   (float width, float height) override;
        VertexData createCylinder(float radius, float height, int slices) override;

    private:
        /**
         * @brief Internal helper to convert a native Raylib Mesh into our agnostic VertexData.
         * @param rayMesh The native Raylib mesh to convert.
         * @return The filled agnostic graphic::VertexData structure.
         */
        VertexData convertRaylibMesh(::Mesh rayMesh);
};

} // namespace graphic::raylib