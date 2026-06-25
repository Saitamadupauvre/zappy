#include "ResourceInstanceBatch.hpp"
#include "behavior/resource/ResourceBehavior.hpp"
#include "behavior/drawable/model/ModelDrawableBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "entity/Entity.hpp"

namespace zappy {

void ResourceInstanceBatch::rebuild(const EntityManager& em)
{
    for (auto& b : _batches)
        b.transforms.clear();

    for (const auto& entity : em.getEntities()) {
        if (entity->getType() != "resource") continue;

        auto drawable = entity->getBehavior<behavior::ModelDrawableBehavior>();
        if (!drawable || !drawable->isVisible()) continue;

        auto res = entity->getBehavior<behavior::ResourceBehavior>();
        auto tf  = entity->getBehavior<behavior::TransformBehavior>();
        if (!res || !tf) continue;

        int type = res->getType();
        TypeBatch& batch = _batches[type];

        if (!batch.initialized) {
            batch.model       = drawable->getModel();
            batch.tint        = drawable->getTint();
            batch.meshTints   = drawable->getMeshTints();
            batch.meshShaders = drawable->getMeshShaders();
            batch.initialized = true;
        }

        batch.transforms.push_back(tf->getMatrix());
    }

    _dirty = false;
}

void ResourceInstanceBatch::draw(graphic::IRenderer& renderer)
{
    for (auto& batch : _batches) {
        if (!batch.initialized || batch.transforms.empty()) continue;
        renderer.drawModelInstanced(
            batch.model, batch.transforms,
            batch.tint, batch.meshTints, batch.meshShaders);
    }
}

} // namespace zappy
