#include "MapGroundFactory.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "behavior/drawable/mesh/GroundDrawableBehavior.hpp"

namespace zappy {

void MapGroundFactory::build(EntityManager& em, graphic::IRenderer& renderer,
                             const graphic::VertexData& meshData, int worldW, int worldH,
                             bool showTiles)
{
    clear(em);
    auto entity = em.createEntity(GROUND_ENTITY_ID, "ground");
    entity->addBehavior<behavior::TransformBehavior>();
    entity->addBehavior<behavior::GroundDrawableBehavior>(renderer, meshData, worldW, worldH, showTiles);
}

void MapGroundFactory::clear(EntityManager& em)
{
    em.removeEntity(GROUND_ENTITY_ID);
}

} // namespace zappy
