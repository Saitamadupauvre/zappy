#pragma once

#include "ISettingsSection.hpp"

namespace zappy {

class CreditsSection : public ISettingsSection {
public:
    std::string sectionTitle() const override { return "Credits"; }

    std::vector<behavior::hud::HudElement> getHudElements() const override
    {
        return {
            {behavior::hud::TextData{"Zappy GUI", 15.f, {220, 230, 255, 255}}},
            {behavior::hud::TextData{"EPITECH Year-End Project 2026", 11.f, {170, 185, 215, 200}}},
            {behavior::hud::TextData{"Built with Raylib + C++20", 11.f, {170, 185, 215, 200}}},
        };
    }
};

} // namespace zappy
