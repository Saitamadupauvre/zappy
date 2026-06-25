#include "TileEntityFactory.hpp"
#include "entity/Entity.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

static constexpr graphic::Color4b TILE_COLOR_A = { 65,  75,  85, 255};
static constexpr graphic::Color4b TILE_COLOR_B = { 75,  85,  95, 255};

void TileEntityFactory::init(graphic::IRenderer& renderer,
                              graphic::IMeshFactory& factory,
                              float spacing)
{
    _spacing = spacing;
    _mesh    = renderer.uploadMesh(factory.createCube(spacing, TILE_HEIGHT, spacing));
}

void TileEntityFactory::spawn(EntityManager& em, graphic::EntityID id,
                               const graphic::Vector3f& pos, graphic::Color4b tint,
                               const graphic::Vector3f& upAt) const
{
    EntityBuilder(em, id, "tile")
        .transform().position(pos)
        .transform().orientation(upAt, 0.0f)
        .drawable().mesh(_mesh, _noTex, tint)
        .interaction().selectable()
        .build();
}

} // namespace zappy
