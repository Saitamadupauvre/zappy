#pragma once

#include "WorldInfoProvider.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/Types.hpp"
#include <memory>

namespace zappy {

class WorldInfoPanel {
public:
    static constexpr graphic::EntityID WORLD_INFO_HUD_ID = 9888;

    void setup(HudManager& hud);

    void show(const WorldInfoProvider::Stats& stats);
    void hide();
    void toggle(const WorldInfoProvider::Stats& stats);
    bool isVisible() const;

private:
    std::shared_ptr<WorldInfoProvider>               _provider;
    std::shared_ptr<behavior::HudContainerBehavior>  _container;
};

} // namespace zappy
