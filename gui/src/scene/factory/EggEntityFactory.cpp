#include "EggEntityFactory.hpp"
#include "entity/Entity.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"
#include <vector>

namespace zappy {

void EggEntityFactory::init(graphic::IRenderer& renderer, graphic::IMeshFactory& factory)
{
    (void)factory;
    _renderer = &renderer;
    _model = renderer.loadModel(EGG_MODEL_PATH);
}

void EggEntityFactory::spawn(EntityManager& em, uint32_t eggId,
                              const graphic::Vector3f& pos,
                              graphic::Color4b color) const
{
    EntityBuilder(em, eggEntityId(eggId), "egg")
        .transform().position(pos)
        .transform().scale(EGG_SCALE)
        .drawable().model(*_renderer, _model, color, {0.f, 0.f, 0.f}, false)
        .egg().data(eggId)
        .build();
}

void EggEntityFactory::clearAll(EntityManager& em) const
{
    std::vector<graphic::EntityID> toRemove;
    for (const auto& e : em.getEntities())
        if (e->getType() == "egg")
            toRemove.push_back(e->getID());
    for (auto id : toRemove)
        em.removeEntity(id);
}

} // namespace zappy
