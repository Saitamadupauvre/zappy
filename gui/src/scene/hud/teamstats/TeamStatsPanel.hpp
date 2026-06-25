#pragma once

#include "TeamStatsProvider.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/Types.hpp"
#include "parser/Resources/Resources.hpp"
#include <memory>
#include <string>

namespace zappy {

class TeamStatsPanel {
public:
    static constexpr graphic::EntityID TEAM_STATS_HUD_ID = 9889;

    void setup(HudManager& hud);

    void show(const std::string& team, int playerCount, int maxLevel, float avgLevel,
              const Resources& totalResources);
    void hide();
    bool isVisible() const;

private:
    std::shared_ptr<TeamStatsProvider>               _provider;
    std::shared_ptr<behavior::HudContainerBehavior>  _container;
};

} // namespace zappy
