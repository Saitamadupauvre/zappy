#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/SpeedControlProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/Types.hpp"
#include "logger/ContextLogger.hpp"
#include <functional>
#include <memory>
#include <string>

namespace zappy {

class SpeedPanel {
public:
    // Entity ID: 9892
    static constexpr graphic::EntityID SPEED_HUD_ID = 9892;

    void setup(HudManager& hud);
    void setSendLine(std::function<void(std::string)> fn);
    void setTimeUnit(int tu);

private:
    std::shared_ptr<SpeedControlProvider>            _provider;
    std::shared_ptr<behavior::HudContainerBehavior>  _container;
    std::function<void(std::string)>                 _sendLine;
    ContextLogger _log{"SpeedPanel"};
};

} // namespace zappy
