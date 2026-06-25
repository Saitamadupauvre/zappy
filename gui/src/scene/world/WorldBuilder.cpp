#include "WorldBuilder.hpp"
#include "scene/layout/IMapLayout.hpp"

namespace zappy {

WorldBuilder::WorldBuilder(EntityManager& entities, TileMap& tileMap)
    : _entities(entities), _tileMap(tileMap)
{}

void WorldBuilder::init(graphic::IRenderer& renderer, graphic::IMeshFactory& meshFactory)
{
    _renderer = &renderer;
    _resourceFactory.init(renderer, meshFactory);
}

void WorldBuilder::build(IMapLayout& layout, int w, int h, bool showTiles)
{
    if (!_renderer) return;

    _tileMap.build(layout, w, h);

    _groundFactory.build(_entities, *_renderer,
                         layout.buildMesh(w, h), w, h, showTiles);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            auto base = _tileMap.tilePos(x, y);
            auto up   = _tileMap.tileUp(x, y);
            _resourceFactory.spawnAll(_entities, x, y, w, base, up);
        }
    }
}

void WorldBuilder::rebuildGroundOnly(IMapLayout& layout, int w, int h, bool showTiles)
{
    if (!_renderer) return;
    _groundFactory.build(_entities, *_renderer,
                         layout.buildMesh(w, h), w, h, showTiles);
}

void WorldBuilder::clear(int w, int h)
{
    _tileMap.clear();
    _groundFactory.clear(_entities);
    _resourceFactory.clearAll(_entities, w, h);
}

} // namespace zappy
