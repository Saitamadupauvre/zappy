#include "RaylibMeshFactory.hpp"
#include <raylib.h>

namespace graphic::raylib {

VertexData RaylibMeshFactory::createCube(float width, float height, float length) {
    ::Mesh rayMesh = GenMeshCube(width, height, length);
    return convertRaylibMesh(rayMesh);
}

VertexData RaylibMeshFactory::createSphere(float radius, int rings, int slices) {
    ::Mesh rayMesh = GenMeshSphere(radius, rings, slices);
    return convertRaylibMesh(rayMesh);
}

VertexData RaylibMeshFactory::createTorus(float radius, float tubeRadius, int radialSegments, int tubularSegments) {
    ::Mesh rayMesh = GenMeshTorus(radius, tubeRadius, radialSegments, tubularSegments);
    return convertRaylibMesh(rayMesh);
}

VertexData RaylibMeshFactory::convertRaylibMesh(::Mesh rayMesh) {
    VertexData data;

    for (int i = 0; i < rayMesh.vertexCount; ++i) {
        float vx = rayMesh.vertices[i * 3];
        float vy = rayMesh.vertices[i * 3 + 1];
        float vz = rayMesh.vertices[i * 3 + 2];
        data.positions.push_back(Vector3f{vx, vy, vz});

        if (rayMesh.normals) {
            float nx = rayMesh.normals[i * 3];
            float ny = rayMesh.normals[i * 3 + 1];
            float nz = rayMesh.normals[i * 3 + 2];
            data.normals.push_back(Vector3f{nx, ny, nz});
        }

        if (rayMesh.texcoords) {
            float u = rayMesh.texcoords[i * 2];
            float v = rayMesh.texcoords[i * 2 + 1];
            data.texCoords.push_back(Vector2f{u, v});
        }
    }

    if (rayMesh.indices) {
        int totalIndices = rayMesh.triangleCount * 3;
        for (int i = 0; i < totalIndices; ++i) {
            data.indices.push_back(rayMesh.indices[i]);
        }
    } else {
        for (int i = 0; i < rayMesh.vertexCount; ++i) {
            data.indices.push_back(static_cast<unsigned short>(i));
        }
    }

    UnloadMesh(rayMesh);

    return data;
}

VertexData RaylibMeshFactory::createPlane(float width, float height) {
    ::Mesh rayMesh = GenMeshPlane(width, height, 1, 1);
    return convertRaylibMesh(rayMesh);
}

VertexData RaylibMeshFactory::createCylinder(float radius, float height, int slices) {
    ::Mesh rayMesh = GenMeshCylinder(radius, height, slices);
    return convertRaylibMesh(rayMesh);
}

} // namespace graphic::raylib