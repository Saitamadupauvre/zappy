#pragma once

#include "graphic/Types.hpp"
#include <cstdint>
#include <variant>
#include <vector>

namespace event {

// Sent by scene/world logic to entity behaviors.
// Behaviors never hold references to game-state objects — they only react to these.

struct EntityMoveToEvent {
    uint32_t          entityId;
    graphic::Vector3f target;
    float             duration; // 0 = instant teleport
};

struct EntityRotateToEvent {
    uint32_t          entityId;
    float             targetYaw;
    float             duration; // 0 = instant
    graphic::Vector3f up      = graphic::Vector3f::up();      // surface normal to align to
    graphic::Vector3f forward = graphic::Vector3f::forward(); // tangent "north" to face
};

struct HoverEvent  { unsigned int entityId; bool isHovered; };
struct SelectEvent { unsigned int entityId; bool isSelected; };
struct ClickEvent  { unsigned int entityId; };
struct EntitySelectedEvent { unsigned int entityId; };
struct CameraFollowToggleEvent {};
struct TeamSelectEvent {
    std::vector<uint32_t> ids;
    bool isSelected;
};

using LogicEvent = std::variant<
    EntityMoveToEvent,
    EntityRotateToEvent,
    HoverEvent,
    SelectEvent,
    ClickEvent,
    EntitySelectedEvent,
    CameraFollowToggleEvent,
    TeamSelectEvent
>;

} // namespace event
