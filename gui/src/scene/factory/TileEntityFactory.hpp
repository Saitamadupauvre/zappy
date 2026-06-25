#pragma once

#include "entity/Entity.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/IMeshFactory.hpp"
#include "graphic/Types.hpp"

namespace zappy {

class EntityManager;

class TileEntityFactory
{
public:
    static constexpr float TILE_HEIGHT = 0.2f;
    // High base so tile IDs never collide with player IDs (raw server ids, small).
    static constexpr graphic::EntityID TILE_BASE_ID = 1u << 23;

    TileEntityFactory() = default;
    void init(graphic::IRenderer& renderer, graphic::IMeshFactory& factory, float spacing);

    void spawn(EntityManager& em, graphic::EntityID id,
               const graphic::Vector3f& pos, graphic::Color4b tint,
               const graphic::Vector3f& upAt = graphic::Vector3f::up()) const;

    static graphic::EntityID tileId(int x, int y, int worldW)
    {
        return TILE_BASE_ID + static_cast<graphic::EntityID>(y * worldW + x);
    }

private:
    graphic::MeshHandle    _mesh{0};
    graphic::TextureHandle _noTex{0};
    float                  _spacing = 2.0f;
};

} // namespace zappy
