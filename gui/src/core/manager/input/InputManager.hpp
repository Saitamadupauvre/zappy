#pragma once

#include "logger/ContextLogger.hpp"
#include "event/Event.hpp"
#include "graphic/Types.hpp"
#include <unordered_map>
#include <vector>
#include <functional>
#include <string>

namespace zappy
{

/**
 * @enum InputAction
 * @brief Abstract input actions mapped to physical keys or mouse inputs.
 */
enum class InputAction {
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    TOGGLE_POV,
    ZOOM_IN,         ///< Triggered by mouse wheel scrolling up.
    ZOOM_OUT,        ///< Triggered by mouse wheel scrolling down.
    UNKNOWN
};

/**
 * @class InputManager
 * @brief Manages abstraction between hardware input events and game actions.
 */
class InputManager
{
    public:
        using ActionListener = std::function<void(bool isActive)>;
        using TriggerListener = std::function<void()>;

        /**
         * @struct MouseStateData
         * @brief Stores the cached positional and raycasting state of the mouse.
         */
        struct MouseStateData {
            graphic::Vector2f screenPosition{0.0f, 0.0f};
        };

        InputManager();
        ~InputManager() = default;

        /**
         * @brief Central routing pipeline for incoming abstract window events.
         * @param event The base event interface to parse.
         */
        void handleEvent(const event::Event& event);

        // Binding configuration
        void bindActionListener(InputAction action, ActionListener listener);
        void bindTriggerListener(InputAction action, TriggerListener listener);
        void rebindAction(InputAction action, graphic::KeyCode newKey);

        // State Queries (Polling)
        bool isActionActive(InputAction action) const;
        std::vector<graphic::KeyCode> getBoundKeys(InputAction action) const;
        
        // Status Getters
        MouseStateData getMouseData() const noexcept { return _mouseData; }
        graphic::MouseBtn getLastMouseButton() const noexcept { return _lastMouseBtn; }

        std::string actionToString(InputAction action) const;

    private:
        ContextLogger _log{"InputManager"};

        std::unordered_map<InputAction, std::vector<graphic::KeyCode>> _keyBindings;

        std::unordered_map<graphic::KeyCode, bool> _keyStates;
        std::unordered_map<InputAction, bool> _actionStates;
    
        std::unordered_map<InputAction, std::vector<ActionListener>> _actionListeners;
        std::unordered_map<InputAction, std::vector<TriggerListener>> _triggerListeners;

        MouseStateData _mouseData;
        graphic::MouseBtn _lastMouseBtn = graphic::MouseBtn::LEFT;

        void initDefaultBindings();
        void addKeyBinding(InputAction action, graphic::KeyCode additionalKey);
        InputAction getActionFromKeyCode(graphic::KeyCode code) const;
        
        void handleKey(graphic::KeyCode code, bool isPressed);
        void handleMouse(const event::MouseButtonEvent& e);
        void handleWheel(float delta);
};

} // namespace zappy