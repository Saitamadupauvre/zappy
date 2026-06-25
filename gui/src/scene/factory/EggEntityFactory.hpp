#pragma once

#include "entity/Entity.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/IMeshFactory.hpp"
#include "graphic/Types.hpp"
#include <cstdint>

namespace zappy {

class EntityManager;

class EggEntityFactory
{
public:
    static constexpr graphic::EntityID EGG_BASE_ID = 1u << 21;

    EggEntityFactory() = default;
    void init(graphic::IRenderer& renderer, graphic::IMeshFactory& factory);

    void spawn(EntityManager& em, uint32_t eggId,
               const graphic::Vector3f& pos,
               graphic::Color4b color) const;

    // Remove every egg entity currently in the scene.
    void clearAll(EntityManager& em) const;

    static graphic::EntityID eggEntityId(uint32_t eggId)
    {
        return EGG_BASE_ID + static_cast<graphic::EntityID>(eggId);
    }

private:
    graphic::IRenderer*  _renderer = nullptr;
    graphic::ModelHandle _model{0};

    static constexpr const char* EGG_MODEL_PATH = "assets/model/egg.glb";
    static constexpr float       EGG_SCALE      = 0.25f;
};

} // namespace zappy
