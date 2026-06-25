#include "behavior/drawable/ADrawable.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "event/Event.hpp"
#include "graphic/IRenderer.hpp"
#include "entity/Entity.hpp"

namespace behavior {

void ADrawable::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::RenderEvent& e) {
            auto t = owner.getBehavior<TransformBehavior>();
            auto matrix = t ? t->getMatrix() : graphic::Matrix4x4::Identity();
            draw(e.renderer, matrix);
        }
    );
}

} // namespace behavior
