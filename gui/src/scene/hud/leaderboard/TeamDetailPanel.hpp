#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/TeamDetailProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/Types.hpp"
#include "logger/ContextLogger.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace zappy {

class TeamDetailPanel {
public:
    // Entity ID: 9896
    static constexpr graphic::EntityID TEAM_DETAIL_HUD_ID = 9896;

    void setup(HudManager& hud);

    void open(const std::string& team, std::vector<PlayerDetailEntry> entries);
    void refreshIfOpen(const std::string& team, std::vector<PlayerDetailEntry> entries);
    void close();

    bool isFullyVisible() const;
    const std::string& getCurrentTeam() const;
    void setOnFollowClick(std::function<void(uint32_t)> cb);

private:
    std::shared_ptr<TeamDetailProvider>              _provider;
    std::shared_ptr<behavior::HudContainerBehavior>  _container;
    ContextLogger _log{"TeamDetailPanel"};
};

} // namespace zappy
