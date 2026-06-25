#pragma once

#include "ISettingsSection.hpp"
#include "graphic/Types.hpp"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string>

namespace zappy {

class PikminClickerSection : public ISettingsSection {
public:
    void setTexture(graphic::TextureHandle tex) { _tex = tex; }

    std::string sectionTitle() const override { return "Pikmin Clicker"; }

    std::vector<behavior::hud::HudElement> getHudElements() const override
    {
        _tick();

        std::vector<behavior::hud::HudElement> elems;

        elems.push_back({behavior::hud::TextData{
            std::to_string(static_cast<long long>(_pikmin)) + " Pikmin",
            18.f, {255, 220, 80, 255}
        }});

        if (_perSecond > 0.f) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.2f per second", _perSecond);
            elems.push_back({behavior::hud::TextData{buf, 11.f, {160, 255, 160, 200}}});
        }

        if (_tex.id != 0) {
            elems.push_back({behavior::hud::ImageButtonData{
                _tex, 96.f, 96.f,
                {255, 255, 255, 255},
                {255, 200, 80, 255},
                1.f,
                [this]() { _pikmin += _perClick; }
            }});
        } else {
            elems.push_back({behavior::hud::ButtonData{
                "Throw Pikmin!", 14.f, 200.f, 50.f,
                {255, 255, 255, 255}, {80, 60, 20, 220}, {160, 120, 40, 235},
                [this]() { _pikmin += _perClick; }
            }});
        }

        {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "+%.0f per throw", _perClick);
            elems.push_back({behavior::hud::TextData{buf, 10.f, {200, 200, 200, 160}}});
        }

        elems.push_back({behavior::hud::TextData{"── Upgrades ──", 11.f, {150, 170, 230, 190}}});

        _pushAutoUpgrade(elems, "Red Pikmin",     10.0,    0.1f,  _redCount);
        _pushAutoUpgrade(elems, "Yellow Pikmin",  60.0,    0.5f,  _yellowCount);
        _pushAutoUpgrade(elems, "Blue Pikmin",    300.0,   3.f,   _blueCount);
        _pushAutoUpgrade(elems, "Rock Pikmin",    2000.0,  15.f,  _rockCount);
        _pushAutoUpgrade(elems, "Winged Pikmin",  10000.0, 80.f,  _wingedCount);

        _pushClickUpgrade(elems, "Captain Olimar", 500.0,  2.f,  _captainBought);
        _pushClickUpgrade(elems, "Onion",          5000.0, 5.f,  _onionBought);

        return elems;
    }

private:
    mutable double _pikmin    = 0.0;
    mutable float  _perClick  = 1.f;
    mutable float  _perSecond = 0.f;
    mutable std::chrono::steady_clock::time_point _lastTick{};

    mutable int  _redCount    = 0;
    mutable int  _yellowCount = 0;
    mutable int  _blueCount   = 0;
    mutable int  _rockCount   = 0;
    mutable int  _wingedCount = 0;
    mutable bool _captainBought = false;
    mutable bool _onionBought   = false;

    graphic::TextureHandle _tex{};

    void _tick() const
    {
        auto now = std::chrono::steady_clock::now();
        if (_lastTick.time_since_epoch().count() != 0) {
            float dt = std::chrono::duration<float>(now - _lastTick).count();
            if (dt > 0.f && dt < 5.f)
                _pikmin += static_cast<double>(_perSecond) * dt;
        }
        _lastTick = now;
    }

    void _pushAutoUpgrade(std::vector<behavior::hud::HudElement>& elems,
                          const char* name, double baseCost, float cpsGain,
                          int& count) const
    {
        double cost = baseCost * std::pow(1.15, count);
        bool canAfford = (_pikmin >= cost);
        char label[128];
        std::snprintf(label, sizeof(label), "%s [%d]  %.0f Pikmin", name, count, cost);
        elems.push_back({behavior::hud::ButtonData{
            label, 11.f, 260.f, 30.f,
            {255, 255, 255, 255},
            canAfford ? graphic::Color4b{35, 90, 45, 210} : graphic::Color4b{45, 45, 45, 150},
            canAfford ? graphic::Color4b{55, 150, 70, 235} : graphic::Color4b{55, 55, 55, 170},
            canAfford ? std::function<void()>([this, cost, cpsGain, &count]() {
                _pikmin -= cost;
                _perSecond += cpsGain;
                ++count;
            }) : std::function<void()>{}
        }});
    }

    void _pushClickUpgrade(std::vector<behavior::hud::HudElement>& elems,
                           const char* name, double cost, float mult,
                           bool& bought) const
    {
        if (bought) {
            char label[128];
            std::snprintf(label, sizeof(label), "%s (owned)  x%.0f click", name, mult);
            elems.push_back({behavior::hud::TextData{label, 10.f, {100, 200, 100, 160}}});
            return;
        }
        bool canAfford = (_pikmin >= cost);
        char label[128];
        std::snprintf(label, sizeof(label), "%s  %.0f Pikmin  (x%.0f click)", name, cost, mult);
        elems.push_back({behavior::hud::ButtonData{
            label, 11.f, 260.f, 30.f,
            {255, 255, 255, 255},
            canAfford ? graphic::Color4b{70, 45, 110, 210} : graphic::Color4b{45, 45, 45, 150},
            canAfford ? graphic::Color4b{120, 75, 190, 235} : graphic::Color4b{55, 55, 55, 170},
            canAfford ? std::function<void()>([this, cost, mult, &bought]() {
                _pikmin -= cost;
                _perClick *= mult;
                bought = true;
            }) : std::function<void()>{}
        }});
    }
};

} // namespace zappy
