#include "InputManager.hpp"
#include <format>

namespace zappy
{

InputManager::InputManager()
{
    initDefaultBindings();
}

void InputManager::initDefaultBindings()
{
    _keyBindings[InputAction::MOVE_FORWARD]  = { graphic::KeyCode::KEY_W, graphic::KeyCode::KEY_UP };
    _keyBindings[InputAction::MOVE_BACKWARD] = { graphic::KeyCode::KEY_S, graphic::KeyCode::KEY_DOWN };
    _keyBindings[InputAction::MOVE_LEFT]     = { graphic::KeyCode::KEY_A, graphic::KeyCode::KEY_LEFT };
    _keyBindings[InputAction::MOVE_RIGHT]    = { graphic::KeyCode::KEY_D, graphic::KeyCode::KEY_RIGHT };
    _keyBindings[InputAction::TOGGLE_POV]    = { graphic::KeyCode::KEY_SPACE };

    for (int i = 0; i < static_cast<int>(InputAction::UNKNOWN); ++i)
        _actionStates[static_cast<InputAction>(i)] = false;
}

std::string InputManager::actionToString(InputAction action) const
{
    switch (action)
    {
    case InputAction::MOVE_FORWARD:  return "MOVE_FORWARD";
    case InputAction::MOVE_BACKWARD: return "MOVE_BACKWARD";
    case InputAction::MOVE_LEFT:     return "MOVE_LEFT";
    case InputAction::MOVE_RIGHT:    return "MOVE_RIGHT";
    case InputAction::TOGGLE_POV:    return "TOGGLE_POV";
    case InputAction::ZOOM_IN:       return "ZOOM_IN";
    case InputAction::ZOOM_OUT:      return "ZOOM_OUT";
    case InputAction::UNKNOWN:       return "UNKNOWN";
    }
    return "INVALID_ACTION";
}

void InputManager::bindActionListener(InputAction action, ActionListener listener)
{
    if (action == InputAction::UNKNOWN) {
        _log.warn("Attempted to bind an action listener to InputAction::UNKNOWN");
        return;
    }
    _actionListeners[action].push_back(listener);
    _log.info(std::format("Bound ActionListener for: {}", actionToString(action)));
}

void InputManager::bindTriggerListener(InputAction action, TriggerListener listener)
{
    if (action == InputAction::UNKNOWN) {
        _log.warn("Attempted to bind a trigger listener to InputAction::UNKNOWN");
        return;
    }
    _triggerListeners[action].push_back(listener);
    _log.info(std::format("Bound TriggerListener for: {}", actionToString(action)));
}

void InputManager::rebindAction(InputAction action, graphic::KeyCode newKey)
{
    if (action == InputAction::UNKNOWN) {
        _log.warn("Cannot rebind InputAction::UNKNOWN.");
        return;
    }
    _keyBindings[action] = { newKey };
    _log.info(std::format("Rebound action {} to key: {}", actionToString(action), static_cast<int>(newKey)));
}

void InputManager::addKeyBinding(InputAction action, graphic::KeyCode additionalKey)
{
    if (action == InputAction::UNKNOWN) return;
    _keyBindings[action].push_back(additionalKey);
}

InputAction InputManager::getActionFromKeyCode(graphic::KeyCode code) const
{
    for (const auto& [action, boundKeys] : _keyBindings) {
        for (graphic::KeyCode key : boundKeys) {
            if (key == code)
                return action;
        }
    }
    return InputAction::UNKNOWN;
}

bool InputManager::isActionActive(InputAction action) const
{
    auto it = _actionStates.find(action);
    if (it == _actionStates.end()) {
        _log.trace(std::format("Polling uninitialized action: {}", actionToString(action)));
        return false;
    }
    return it->second;
}

void InputManager::handleEvent(const event::Event& ev)
{
    event::on(ev,
        [&](const event::KeyEvent& e)         { handleKey(e.key, e.pressed); },
        [&](const event::MouseButtonEvent& e)  { handleMouse(e); },
        [&](const event::MouseWheelEvent& e)   { handleWheel(e.delta); }
    );
}

void InputManager::handleKey(graphic::KeyCode code, bool isPressed)
{
    InputAction action = getActionFromKeyCode(code);
    if (action == InputAction::UNKNOWN)
        return;

    _keyStates[code] = isPressed;

    bool isActionNowActive = false;
    for (graphic::KeyCode boundKey : _keyBindings[action]) {
        if (_keyStates[boundKey]) {
            isActionNowActive = true;
            break;
        }
    }

    bool previousActionState = _actionStates[action];
    if (previousActionState == isActionNowActive)
        return;

    _actionStates[action] = isActionNowActive;

    _log.debug(std::format("Action {} {} -> {}",
        actionToString(action),
        (previousActionState ? "DOWN" : "UP"),
        (isActionNowActive ? "DOWN" : "UP")));

    if (_actionListeners.count(action)) {
        for (const auto& cb : _actionListeners[action])
            cb(isActionNowActive);
    }

    if (isActionNowActive && !previousActionState && _triggerListeners.count(action)) {
        _log.info(std::format("Trigger: {}", actionToString(action)));
        for (const auto& cb : _triggerListeners[action])
            cb();
    }
}

void InputManager::handleMouse(const event::MouseButtonEvent& e)
{
    _mouseData.screenPosition = e.screenPos;
    _lastMouseBtn             = e.button;

    _log.info(std::format("Mouse [btn={} pressed={}] screen=({:.1f},{:.1f})",
        static_cast<int>(e.button), e.pressed,
        e.screenPos.x, e.screenPos.y));
}

void InputManager::handleWheel(float delta)
{
    if (delta == 0.0f) return;

    InputAction action = (delta > 0.0f) ? InputAction::ZOOM_IN : InputAction::ZOOM_OUT;
    _log.debug(std::format("Wheel {:.1f} -> {}", delta, actionToString(action)));

    if (_triggerListeners.count(action)) {
        for (const auto& cb : _triggerListeners[action])
            cb();
    }
}

std::vector<graphic::KeyCode> InputManager::getBoundKeys(InputAction action) const
{
    auto it = _keyBindings.find(action);
    if (it == _keyBindings.end()) return {};
    return it->second;
}

} // namespace zappy
