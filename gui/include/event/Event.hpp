#pragma once

#include "graphic/Types.hpp"
#include "util/Overloaded.hpp"
#include <variant>

namespace graphic { class IRenderer; }

namespace event {

struct KeyEvent         { graphic::KeyCode key; bool pressed; };
struct MouseButtonEvent { graphic::MouseBtn button; bool pressed;
                          graphic::Vector2f screenPos; };
struct MouseMoveEvent   { graphic::Vector2f delta; };
struct MouseWheelEvent  { float delta; };
struct WindowClosedEvent  {};
struct WindowResizedEvent { graphic::Vector2f newSize; };
struct RenderEvent        { graphic::IRenderer& renderer; graphic::Vector2f viewportSize; };

using Event = std::variant<
    KeyEvent,
    MouseButtonEvent,
    MouseMoveEvent,
    MouseWheelEvent,
    WindowClosedEvent,
    WindowResizedEvent,
    RenderEvent
>;

template<typename... Handlers>
auto on(const Event& ev, Handlers&&... handlers)
{
    return std::visit(overloaded{
        std::forward<Handlers>(handlers)...,
        [](auto&&) {}
    }, ev);
}

} // namespace event
