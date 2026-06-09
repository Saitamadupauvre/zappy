#include "behavior/drawable/ADrawable.hpp"
#include "event/Event.hpp"
#include "graphic/IRenderer.hpp"
#include "entity/Entity.hpp"

namespace behavior {

void ADrawable::onEvent([[maybe_unused]] graphic::Entity& owner, const event::Event& ev)
{
    event::on(ev,
        [&](const event::RenderEvent& e) {
            draw(e.renderer, graphic::Matrix4x4::Identity());
        }
    );
}

} // namespace behavior
