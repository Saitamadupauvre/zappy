#pragma once

#include "graphic/IMeshFactory.hpp"
#include "graphic/IRenderer.hpp"
#include "scene/factory/MapGroundFactory.hpp"
#include "scene/factory/ResourceEntityFactory.hpp"
#include "scene/TileMap.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "logger/ContextLogger.hpp"

namespace zappy {

class IMapLayout;

class WorldBuilder {
public:
    WorldBuilder(EntityManager& entities, TileMap& tileMap);

    void init(graphic::IRenderer& renderer, graphic::IMeshFactory& meshFactory);
    void build(IMapLayout& layout, int w, int h, bool showTiles);
    void rebuildGroundOnly(IMapLayout& layout, int w, int h, bool showTiles);
    void clear(int w, int h);

private:
    EntityManager&       _entities;
    TileMap&             _tileMap;
    ResourceEntityFactory _resourceFactory;
    MapGroundFactory      _groundFactory;
    graphic::IRenderer*  _renderer = nullptr;
    ContextLogger _log{"WorldBuilder"};
};

} // namespace zappy
