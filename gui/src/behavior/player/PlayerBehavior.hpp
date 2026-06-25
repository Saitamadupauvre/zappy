#pragma once

#include "behavior/ABehavior.hpp"
#include "world/WorldTypes.hpp"
#include <cstdint>
#include <string>

namespace behavior {

class PlayerBehavior : public ABehavior
{
public:
    explicit PlayerBehavior(const zappy::PlayerState& state);

    void onUpdate(graphic::Entity&, float) override {}
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

    uint32_t                 playerId()    const { return _playerId; }
    int                      x()           const { return _x; }
    int                      y()           const { return _y; }
    int                      orientation() const { return _orientation; }
    int                      level()       const { return _level; }
    const std::string&       team()        const { return _team; }
    const zappy::Resources&  inventory()   const { return _inventory; }

private:
    uint32_t          _playerId;
    int               _x;
    int               _y;
    int               _orientation;
    int               _level;
    std::string       _team;
    zappy::Resources  _inventory;
};

} // namespace behavior
