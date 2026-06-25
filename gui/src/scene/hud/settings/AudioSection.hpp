#pragma once

#include "ISettingsSection.hpp"

namespace zappy {

class AudioSection : public ISettingsSection {
public:
    std::string sectionTitle() const override { return "Audio"; }

    std::vector<behavior::hud::HudElement> getHudElements() const override
    {
        return {
            {behavior::hud::TextData{"Audio settings coming soon.", 12.f, {180, 190, 220, 200}}}
        };
    }
};

} // namespace zappy
