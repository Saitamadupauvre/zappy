#pragma once

#include "ISettingsSection.hpp"
#include "i18n/I18n.hpp"

namespace zappy {

class CreditsSection : public ISettingsSection {
public:
    std::string sectionTitle() const override { return i18n::tr(i18n::key::SEC_CREDITS); }

    std::vector<behavior::hud::HudElement> getHudElements() const override
    {
        return {
            {behavior::hud::TextData{i18n::tr(i18n::key::CREDITS_TITLE), 15.f, {220, 230, 255, 255}}},
            {behavior::hud::TextData{i18n::tr(i18n::key::CREDITS_SUB1), 11.f, {170, 185, 215, 200}}},
            {behavior::hud::TextData{i18n::tr(i18n::key::CREDITS_SUB2), 11.f, {170, 185, 215, 200}}},
        };
    }
};

} // namespace zappy
