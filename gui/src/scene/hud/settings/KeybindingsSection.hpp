#pragma once

#include "ISettingsSection.hpp"
#include "core/manager/input/InputManager.hpp"
#include "i18n/I18n.hpp"
#include <optional>

namespace zappy {

class KeybindingsSection : public ISettingsSection {
public:
    explicit KeybindingsSection(InputManager& input);

    std::string sectionTitle() const override { return i18n::tr(i18n::key::SEC_KEYBINDINGS); }
    std::vector<behavior::hud::HudElement> getHudElements() const override;

private:
    enum class PendingMode { Rebind, Add };

    InputManager& _input;
    mutable std::optional<InputAction> _pendingAction;
    mutable PendingMode                _pendingMode{PendingMode::Rebind};

    static std::string actionDisplayName(InputAction action);
    static std::string keyDisplayName(const InputKey& key);
    void startCapture(InputAction action, PendingMode mode) const;
};

} // namespace zappy
