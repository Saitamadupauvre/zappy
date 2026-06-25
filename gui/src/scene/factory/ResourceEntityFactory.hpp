#pragma once

#include "entity/Entity.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/IMeshFactory.hpp"
#include "graphic/Types.hpp"

namespace zappy {

class EntityManager;

class ResourceEntityFactory
{
public:
    static constexpr graphic::EntityID RESOURCE_BASE_ID = 1u << 20;

    ResourceEntityFactory() = default;
    ~ResourceEntityFactory();
    void init(graphic::IRenderer& renderer, graphic::IMeshFactory& factory);

    // tileBase: surface center of tile; upAt: outward surface normal.
    void spawnAll(EntityManager& em, int x, int y, int worldW,
                  const graphic::Vector3f& tileBase,
                  const graphic::Vector3f& upAt = graphic::Vector3f::up()) const;

    // Remove every resource entity of a worldW×worldH map.
    void clearAll(EntityManager& em, int worldW, int worldH) const;

    static graphic::EntityID resourceId(int x, int y, int worldW, int slot)
    {
        return RESOURCE_BASE_ID + static_cast<graphic::EntityID>((y * worldW + x) * 7 + slot);
    }

private:
    graphic::IRenderer*  _renderer = nullptr;
    graphic::ModelHandle _model{0};
    graphic::ModelHandle _foodModel{0};

    static constexpr const char* MODEL_PATH      = "assets/model/crystal.glb";
    static constexpr const char* FOOD_MODEL_PATH = "assets/model/apple.glb";
};

} // namespace zappy
