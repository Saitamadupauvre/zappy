#include "RaylibWindow.hpp"
#include <string>

namespace graphic::raylib {

RaylibWindow::RaylibWindow() = default;

RaylibWindow::~RaylibWindow()
{
    if (IsWindowReady()) CloseWindow();
}

void RaylibWindow::create(int width, int height, const std::string& title, int targetFps)
{
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, title.c_str());
    SetTargetFPS(targetFps);

}

bool     RaylibWindow::isOpen()       const { return !_closed; }
void     RaylibWindow::close()              { _closed = true; }
float    RaylibWindow::getDeltaTime() const { return GetFrameTime(); }
Vector2f RaylibWindow::getSize()      const { return { static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()) }; }
void     RaylibWindow::beginFrame()         { if (!_closed) BeginDrawing(); }
void     RaylibWindow::endFrame()           { if (!_closed) EndDrawing(); }

static KeyCode mapRaylibKey(int k)
{
    switch (k) {
        case KEY_ESCAPE:    return KeyCode::KEY_ESCAPE;
        case KEY_SPACE:     return KeyCode::KEY_SPACE;
        case KEY_ENTER:     return KeyCode::KEY_ENTER;
        case KEY_BACKSPACE: return KeyCode::KEY_BACKSPACE;
        case KEY_UP:        return KeyCode::KEY_UP;
        case KEY_DOWN:      return KeyCode::KEY_DOWN;
        case KEY_LEFT:      return KeyCode::KEY_LEFT;
        case KEY_RIGHT:     return KeyCode::KEY_RIGHT;
        default: break;
    }
    if (k >= KEY_A && k <= KEY_Z)
        return static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_A) + (k - KEY_A));
    return KeyCode::UNKNOWN;
}

static event::MouseButtonEvent makeMouseEvent(MouseBtn btn, bool pressed)
{
    ::Vector2 pos = GetMousePosition();
    return { btn, pressed, { pos.x, pos.y } };
}

std::vector<event::Event> RaylibWindow::pollEvents()
{
    std::vector<event::Event> events;
    if (_closed) return events;

    Vector2f currentSize = getSize();
    if (IsWindowResized() || currentSize.x != _lastSize.x || currentSize.y != _lastSize.y) {
        _lastSize = currentSize;
        events.push_back(event::WindowResizedEvent{currentSize});
    }

    if (WindowShouldClose()) {
        events.push_back(event::WindowClosedEvent{});
        return events;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        events.push_back(makeMouseEvent( MouseBtn::LEFT, true));
    else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        events.push_back(makeMouseEvent( MouseBtn::LEFT, false));

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        events.push_back(makeMouseEvent( MouseBtn::RIGHT, true));
    else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
        events.push_back(makeMouseEvent( MouseBtn::RIGHT, false));

    int k;
    while ((k = GetKeyPressed()) > 0) {
        KeyCode code = mapRaylibKey(k);
        if (code != KeyCode::UNKNOWN)
            events.push_back(event::KeyEvent{code, true});
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f)
        events.push_back(event::MouseWheelEvent{wheel});

    ::Vector2 delta = GetMouseDelta();
    if (delta.x != 0.0f || delta.y != 0.0f)
        events.push_back(event::MouseMoveEvent{{delta.x, delta.y}});

    return events;
}

} // namespace graphic::raylib
