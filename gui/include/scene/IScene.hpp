#pragma once

#include "event/Event.hpp"
#include "graphic/IRenderer.hpp"
#include "world/World.hpp"

namespace zappy {

class IScene
{
    public:
        virtual ~IScene() = default;

        virtual void update(const World& world) = 0;
        virtual void render(graphic::IRenderer& renderer) = 0;
        virtual void handleEvent(const event::Event& event) = 0;
};

} // namespace zappy
