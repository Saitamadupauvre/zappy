#pragma once

#include "graphic/Types.hpp"
#include <variant>

namespace event {

struct KeyEvent         { graphic::KeyCode key; bool pressed; };
struct CharInputEvent   { int codepoint; };
struct MouseButtonEvent { graphic::MouseBtn button; bool pressed;
                          graphic::Vector2f screenPos; };
struct MouseMoveEvent   { graphic::Vector2f position; graphic::Vector2f delta; };
struct MouseWheelEvent  { float delta; };
struct WindowClosedEvent  {};
struct WindowResizedEvent { graphic::Vector2f newSize; };

using WindowEvent = std::variant<
    KeyEvent, CharInputEvent, MouseButtonEvent, MouseMoveEvent,
    MouseWheelEvent, WindowClosedEvent, WindowResizedEvent
>;

} // namespace event
