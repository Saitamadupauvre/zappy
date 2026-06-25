#pragma once
#include "event/Event.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "logger/ContextLogger.hpp"


namespace zappy {

    class Scene;

    class HudPicker {
    public:
    bool tryHandleMouseMove(Scene& scene, const graphic::Vector2f& mousePos);
    bool tryHandleClick(Scene& scene, const event::MouseButtonEvent& e);
    bool isWheelConsumed(Scene& scene, const graphic::Vector2f& mousePos) const;

    private:
        static bool isMouseOver(const graphic::Vector2f& mousePos, 
                                const graphic::Vector2f& pos, 
                                const graphic::Vector2f& size);

        ContextLogger _logger{"HudPicker"};
    };
}