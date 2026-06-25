#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/ClockProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/Types.hpp"
#include <functional>
#include <memory>
#include <string>

namespace zappy {

class ClockPanel {
public:
    // Entity ID: 9889
    static constexpr graphic::EntityID CLOCK_HUD_ID = 9889;

    void setup(HudManager& hud);
    void setUptime(unsigned long ticks);
    void setTimeUnit(int tu);
    void tick(float dt);

private:
    std::shared_ptr<ClockProvider>                    _provider;
    std::shared_ptr<behavior::HudContainerBehavior>   _container;
    int _timeUnit = 1;
};

} // namespace zappy
