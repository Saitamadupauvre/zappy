#pragma once

#include "behavior/ABehavior.hpp"

namespace behavior {

// Reacts to TileChangedEvent for its tile, toggling MeshDrawableBehavior
// visibility based on the resource count at its slot index.
class ResourceBehavior : public ABehavior
{
public:
    ResourceBehavior(int tileX, int tileY, int resourceIdx);

    void onUpdate(graphic::Entity&, float) override {}
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

    int getX() const { return _tileX; }
    int getY() const { return _tileY; }
    int getType() const { return _resourceIdx; }
    int getCount() const { return _count; }

    std::string getTypeName() const {
        static const std::string names[] = {
            "Food", "Linemate", "Deraumere", "Sibur", "Mendiane", "Phiras", "Thystame"
        };
        return names[_resourceIdx];
    }

private:
    int _tileX;
    int _tileY;
    int _resourceIdx;
    int _count = 0;
};

} // namespace behavior
