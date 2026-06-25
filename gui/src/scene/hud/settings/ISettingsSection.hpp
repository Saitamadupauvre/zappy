#pragma once

#include "hud/HudElements.hpp"
#include <string>
#include <vector>

namespace zappy {

class ISettingsSection {
public:
    virtual ~ISettingsSection() = default;

    virtual std::string sectionTitle() const = 0;
    virtual std::vector<behavior::hud::HudElement> getHudElements() const = 0;
};

} // namespace zappy
