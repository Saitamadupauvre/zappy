#pragma once

#include "world/WorldTypes.hpp"
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace event {

struct WorldResizedEvent            { int width; int height; };
struct TileChangedEvent             { int x; int y; zappy::Resources resources; };
struct PlayerAddedEvent             { zappy::PlayerState player; };
struct PlayerMovedEvent             { uint32_t id; int x; int y; int orientation; };
struct PlayerLevelChangedEvent      { uint32_t id; int level; };
struct PlayerInventoryChangedEvent  { uint32_t id; zappy::Resources inventory; };
struct PlayerRemovedEvent           { uint32_t id; };
struct PlayerExpelledEvent          { uint32_t id; };
struct PlayerBroadcastEvent         { uint32_t id; std::string message; };
struct IncantationStartEvent        { int x; int y; int level; std::vector<uint32_t> playerIds; };
struct IncantationEndEvent          { int x; int y; bool success; };
struct EggLayingEvent               { uint32_t playerId; };
struct ResourceDroppedEvent         { uint32_t playerId; int resourceIdx; };
struct ResourceCollectedEvent       { uint32_t playerId; int resourceIdx; };
struct EggAddedEvent                { zappy::EggState egg; };
struct EggRemovedEvent              { uint32_t id; };
struct EggHatchedEvent              { uint32_t eggId; };
struct TeamAddedEvent               { std::string name; };
struct TimeUnitChangedEvent         { int timeUnit; };
struct GameEndedEvent               { std::string winnerTeam; };
struct ServerMessageEvent           { std::string message; };
struct MapLayoutCycleEvent          {};
struct TileShadingToggleEvent       {};
struct ServerUptimeEvent            { unsigned long uptimeSeconds; };

using WorldEvent = std::variant<
    WorldResizedEvent,
    TileChangedEvent,
    PlayerAddedEvent,
    PlayerMovedEvent,
    PlayerLevelChangedEvent,
    PlayerInventoryChangedEvent,
    PlayerRemovedEvent,
    PlayerExpelledEvent,
    PlayerBroadcastEvent,
    IncantationStartEvent,
    IncantationEndEvent,
    EggLayingEvent,
    ResourceDroppedEvent,
    ResourceCollectedEvent,
    EggAddedEvent,
    EggRemovedEvent,
    EggHatchedEvent,
    TeamAddedEvent,
    TimeUnitChangedEvent,
    GameEndedEvent,
    ServerMessageEvent,
    MapLayoutCycleEvent,
    TileShadingToggleEvent,
    ServerUptimeEvent
>;

} // namespace event
