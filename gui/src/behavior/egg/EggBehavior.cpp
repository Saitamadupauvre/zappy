#include "EggBehavior.hpp"
#include "behavior/drawable/mesh/MeshDrawableBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"
#include <cmath>

namespace behavior {

EggBehavior::EggBehavior(uint32_t eggId)
    : _eggId(eggId) {}

void EggBehavior::onUpdate(graphic::Entity& owner, float dt)
{
    _time += dt;

    if (_pendingRemoval) {
        _removeTimer -= dt;
        float s = std::max(0.0f, _removeTimer / 0.4f);
        auto t = owner.getBehavior<TransformBehavior>();
        if (t) t->setScale({s, s, s});
        return;
    }

    float pulse = 0.9f + 0.1f * std::sin(_time * 2.0f * M_PI * 1.5f);
    auto t = owner.getBehavior<TransformBehavior>();
    if (t) t->setScale({pulse, pulse, pulse});
}

void EggBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::EggHatchedEvent& e) {
            if (e.eggId != _eggId) return;
            _pendingRemoval = true;
            _removeTimer    = 0.4f;
            auto drawable = owner.getBehavior<MeshDrawableBehavior>();
            if (drawable) drawable->setTint({255, 220, 50, 255});
        },
        [&](const event::EggRemovedEvent& e) {
            if (e.id != _eggId) return;
            _pendingRemoval = true;
            _removeTimer    = 0.4f;
        }
    );
}

} // namespace behavior
