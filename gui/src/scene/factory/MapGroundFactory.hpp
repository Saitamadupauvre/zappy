#pragma once

#include "entity/Entity.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/Types.hpp"

namespace zappy {

class EntityManager;

// Builds the single grass-covered ground entity from a layout-supplied surface mesh
// (flat grid box, torus, …). Layout-agnostic: it only consumes the mesh.
class MapGroundFactory
{
public:
    static constexpr graphic::EntityID GROUND_ENTITY_ID = 1u << 28;

    void build(EntityManager& em, graphic::IRenderer& renderer,
               const graphic::VertexData& meshData, int worldW, int worldH,
               bool showTiles);
    void clear(EntityManager& em);
};

} // namespace zappy
