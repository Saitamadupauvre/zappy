#include "ResourceBehavior.hpp"
#include "behavior/drawable/model/ModelDrawableBehavior.hpp"
#include "event/Event.hpp"
#include "entity/Entity.hpp"

namespace behavior {

ResourceBehavior::ResourceBehavior(int tileX, int tileY, int resourceIdx)
    : _tileX(tileX), _tileY(tileY), _resourceIdx(resourceIdx) {}

void ResourceBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::TileChangedEvent& e) {
            if (e.x != _tileX || e.y != _tileY) return;
            const int counts[7] = {
                e.resources.food, e.resources.linemate, e.resources.deraumere,
                e.resources.sibur, e.resources.mendiane, e.resources.phiras,
                e.resources.thystame
            };
            _count = counts[_resourceIdx];
            auto drawable = owner.getBehavior<ModelDrawableBehavior>();
            bool vis = _count > 0;
            if (drawable) drawable->setVisible(vis);
            owner.setDormant(!vis);
        }
    );
}

} // namespace behavior
