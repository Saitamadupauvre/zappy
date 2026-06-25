#include "KeybindingsSection.hpp"

namespace zappy {

static constexpr std::pair<InputAction, const char*> REBINDABLE_ACTIONS[] = {
    {InputAction::MOVE_FORWARD,        "Move Forward"},
    {InputAction::MOVE_BACKWARD,       "Move Backward"},
    {InputAction::MOVE_LEFT,           "Move Left"},
    {InputAction::MOVE_RIGHT,          "Move Right"},
    {InputAction::TOGGLE_POV,          "Toggle POV"},
    {InputAction::CYCLE_LAYOUT,        "Cycle Layout"},
    {InputAction::TOGGLE_TILES,        "Toggle Tiles"},
    {InputAction::FOLLOW_TOGGLE,       "Follow Player"},
    {InputAction::TOGGLE_LEADERBOARD,  "Leaderboard"},
    {InputAction::TOGGLE_SETTINGS,     "Settings"},
};

KeybindingsSection::KeybindingsSection(InputManager& input) : _input(input) {}

std::string KeybindingsSection::actionDisplayName(InputAction action)
{
    for (auto& [a, name] : REBINDABLE_ACTIONS)
        if (a == action) return name;
    return "Unknown";
}

std::string KeybindingsSection::keyDisplayName(const InputKey& key)
{
    if (std::holds_alternative<graphic::KeyCode>(key)) {
        switch (std::get<graphic::KeyCode>(key)) {
        case graphic::KeyCode::KEY_W:         return "W";
        case graphic::KeyCode::KEY_A:         return "A";
        case graphic::KeyCode::KEY_S:         return "S";
        case graphic::KeyCode::KEY_D:         return "D";
        case graphic::KeyCode::KEY_Q:         return "Q";
        case graphic::KeyCode::KEY_E:         return "E";
        case graphic::KeyCode::KEY_F:         return "F";
        case graphic::KeyCode::KEY_G:         return "G";
        case graphic::KeyCode::KEY_H:         return "H";
        case graphic::KeyCode::KEY_I:         return "I";
        case graphic::KeyCode::KEY_J:         return "J";
        case graphic::KeyCode::KEY_K:         return "K";
        case graphic::KeyCode::KEY_L:         return "L";
        case graphic::KeyCode::KEY_M:         return "M";
        case graphic::KeyCode::KEY_N:         return "N";
        case graphic::KeyCode::KEY_O:         return "O";
        case graphic::KeyCode::KEY_P:         return "P";
        case graphic::KeyCode::KEY_R:         return "R";
        case graphic::KeyCode::KEY_T:         return "T";
        case graphic::KeyCode::KEY_U:         return "U";
        case graphic::KeyCode::KEY_V:         return "V";
        case graphic::KeyCode::KEY_X:         return "X";
        case graphic::KeyCode::KEY_Y:         return "Y";
        case graphic::KeyCode::KEY_Z:         return "Z";
        case graphic::KeyCode::KEY_B:         return "B";
        case graphic::KeyCode::KEY_C:         return "C";
        case graphic::KeyCode::KEY_SPACE:     return "Space";
        case graphic::KeyCode::KEY_ENTER:     return "Enter";
        case graphic::KeyCode::KEY_ESCAPE:    return "Esc";
        case graphic::KeyCode::KEY_TAB:       return "Tab";
        case graphic::KeyCode::KEY_UP:        return "Up";
        case graphic::KeyCode::KEY_DOWN:      return "Down";
        case graphic::KeyCode::KEY_LEFT:      return "Left";
        case graphic::KeyCode::KEY_RIGHT:     return "Right";
        case graphic::KeyCode::KEY_BACKSPACE: return "Backspace";
        default: return "?";
        }
    }
    if (std::holds_alternative<graphic::MouseBtn>(key)) {
        switch (std::get<graphic::MouseBtn>(key)) {
        case graphic::MouseBtn::LEFT:   return "LMB";
        case graphic::MouseBtn::RIGHT:  return "RMB";
        case graphic::MouseBtn::MIDDLE: return "MMB";
        }
    }
    return "?";
}

void KeybindingsSection::startCapture(InputAction action, PendingMode mode) const
{
    _pendingAction = action;
    _pendingMode   = mode;
    _input.captureNextKey([this, action, mode](graphic::KeyCode key) {
        if (mode == PendingMode::Rebind)
            _input.rebindAction(action, key);
        else
            _input.addKeyBinding(action, key);
        _pendingAction = std::nullopt;
    });
}

std::vector<behavior::hud::HudElement> KeybindingsSection::getHudElements() const
{
    std::vector<behavior::hud::HudElement> elems;

    for (auto& [action, _] : REBINDABLE_ACTIONS) {
        bool isPending = (_pendingAction == action);

        std::string keyLabel;
        if (isPending) {
            keyLabel = (_pendingMode == PendingMode::Rebind)
                ? "- press any key..."
                : "+ press any key...";
        } else {
            auto keys = _input.getBoundKeys(action);
            for (std::size_t i = 0; i < keys.size(); ++i) {
                if (i > 0) keyLabel += "  /  ";
                keyLabel += keyDisplayName(keys[i]);
            }
            if (keyLabel.empty()) keyLabel = "-";
        }

        graphic::Color4b nameColor = isPending
            ? graphic::Color4b{255, 220, 80, 255}
            : graphic::Color4b{210, 220, 245, 255};
        graphic::Color4b keyColor = isPending
            ? graphic::Color4b{255, 200, 60, 200}
            : graphic::Color4b{170, 185, 215, 190};

        elems.push_back({behavior::hud::TextData{actionDisplayName(action), 12.f, nameColor}});
        elems.push_back({behavior::hud::TextData{keyLabel, 10.5f, keyColor}});
        elems.push_back({behavior::hud::ButtonData{
            isPending ? "..." : "Rebind", 11.f, 120.f, 22.f,
            {255, 255, 255, 255}, {50, 75, 140, 210}, {80, 120, 210, 235},
            [this, action, isPending]() {
                if (!isPending) startCapture(action, PendingMode::Rebind);
            }
        }});
        elems.push_back({behavior::hud::ButtonData{
            isPending ? "..." : "+ Add key", 11.f, 120.f, 22.f,
            {255, 255, 255, 255}, {35, 90, 60, 210}, {55, 140, 90, 235},
            [this, action, isPending]() {
                if (!isPending) startCapture(action, PendingMode::Add);
            }
        }});
        elems.push_back({behavior::hud::RectData{260.f, 1.f, {80, 90, 130, 120}, {80, 90, 130, 120}}});
    }

    return elems;
}

} // namespace zappy
