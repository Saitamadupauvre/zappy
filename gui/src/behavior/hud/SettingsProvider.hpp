#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "scene/hud/settings/ISettingsSection.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace behavior::hud {

class SettingsProvider : public IHudProvider {
public:
    void addSection(std::shared_ptr<zappy::ISettingsSection> section)
    {
        _sections.push_back(std::move(section));
    }

    void setOnNavigate(std::function<void()> cb) { _onNavigate = std::move(cb); }
    void setOnQuit(std::function<void()> cb)     { _onQuit = std::move(cb); }

    const std::vector<HudElement>& getHudElements() const override
    {
        _cache.clear();
        if (!_currentSection.has_value()) {
            auto elems = mainMenuElements();
            _cache.insert(_cache.end(), elems.begin(), elems.end());
        } else {
            auto elems = sectionElements(*_currentSection);
            _cache.insert(_cache.end(), elems.begin(), elems.end());
        }
        return _cache;
    }

private:
    std::vector<HudElement> mainMenuElements() const
    {
        std::vector<HudElement> elems;
        for (std::size_t i = 0; i < _sections.size(); ++i) {
            elems.push_back({ButtonData{
                _sections[i]->sectionTitle(), 13.f, 260.f, 38.f,
                {255, 255, 255, 255}, {40, 50, 110, 210}, {70, 90, 180, 235},
                [this, i]() {
                    _currentSection = i;
                    if (_onNavigate) _onNavigate();
                }
            }});
        }
        elems.push_back({ButtonData{
            "Quit Game", 13.f, 260.f, 38.f,
            {255, 255, 255, 255}, {110, 30, 30, 210}, {200, 60, 60, 235},
            [this]() { if (_onQuit) _onQuit(); }
        }});
        return elems;
    }

    std::vector<HudElement> sectionElements(std::size_t idx) const
    {
        std::vector<HudElement> elems;
        elems.push_back({ButtonData{
            "< Back", 11.f, 260.f, 28.f,
            {255, 255, 255, 255}, {70, 45, 45, 200}, {150, 70, 70, 230},
            [this]() {
                _currentSection = std::nullopt;
                if (_onNavigate) _onNavigate();
            }
        }});
        auto sectionElems = _sections[idx]->getHudElements();
        elems.insert(elems.end(), sectionElems.begin(), sectionElems.end());
        return elems;
    }

    std::vector<std::shared_ptr<zappy::ISettingsSection>> _sections;
    mutable std::optional<std::size_t> _currentSection;
    mutable std::vector<HudElement> _cache;
    std::function<void()> _onNavigate;
    std::function<void()> _onQuit;
};

} // namespace behavior::hud
