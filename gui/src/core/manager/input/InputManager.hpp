#pragma once

#include "logger/ContextLogger.hpp"
#include "event/Event.hpp"
#include "graphic/Types.hpp"
#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <variant>

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
    ZOOM_OUT,
    CLICK,
    CYCLE_LAYOUT,    ///< Cycle through map layout modes (Grid → Torus → …).
    TOGGLE_TILES,    ///< Toggle tile-grid shading on/off (off = Perlin variation only).
    FOLLOW_TOGGLE,   ///< Follow selected player / unfollow / free camera.
    TOGGLE_LEADERBOARD,  ///< Show/hide the team leaderboard overlay.
    TOGGLE_SETTINGS,     ///< Show/hide the settings panel.
    ESCAPE,              ///< Context-sensitive escape: close top HUD or open settings.
    UNKNOWN
};

using InputKey = std::variant<graphic::KeyCode, graphic::MouseBtn>;

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
        void addKeyBinding(InputAction action, graphic::KeyCode additionalKey);

        // State Queries (Polling)
        bool isActionActive(InputAction action) const;
        std::vector<InputKey> getBoundKeys(InputAction action) const; 
       
        // Status Getters
        MouseStateData getMouseData() const noexcept { return _mouseData; }
        graphic::MouseBtn getLastMouseButton() const noexcept { return _lastMouseBtn; }

        std::string actionToString(InputAction action) const;

        // Capture the next keyboard key press and deliver it to cb; clears after one use.
        void captureNextKey(std::function<void(graphic::KeyCode)> cb);

    private:
        ContextLogger _log{"InputManager"};

        std::unordered_map<InputAction, std::vector<InputKey>> _keyBindings;
        
        std::unordered_map<graphic::KeyCode, bool> _keyStates;
        std::unordered_map<InputAction, bool> _actionStates;
    
        std::unordered_map<InputAction, std::vector<ActionListener>> _actionListeners;
        std::unordered_map<InputAction, std::vector<TriggerListener>> _triggerListeners;

        MouseStateData _mouseData;
        graphic::MouseBtn _lastMouseBtn = graphic::MouseBtn::LEFT;
        std::function<void(graphic::KeyCode)> _pendingKeyCapture;

        void initDefaultBindings();
        InputAction getActionFromKeyCode(graphic::KeyCode code) const;
        
        void handleKey(graphic::KeyCode code, bool isPressed);
        void handleMouse(const event::MouseButtonEvent& e);
        void handleWheel(float delta);
};

} // namespace zappy