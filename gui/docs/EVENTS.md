# Event System

## Structure

`event::Event` is a **nested variant**:

```
Event = variant<WindowEvent, RenderEvent, WorldEvent, LogicEvent>

WindowEvent = variant<ResizeEvent, CloseEvent, MouseButtonEvent,
                      MouseMoveEvent, MouseWheelEvent, KeyEvent>
RenderEvent = variant<Draw3DEvent, Draw2DEvent>
WorldEvent  = variant<PlayerAddedEvent, TileChangedEvent, ...>
LogicEvent  = variant<EntityMoveToEvent, EntitySelectedEvent, ...>
```

The nested structure means you can match on any leaf type without knowing which outer variant it's in.

## Handling events — event::on()

`event::on()` recurses through nested variants automatically:

```cpp
#include "event/Event.hpp"

void MyBehavior::onEvent(graphic::Entity& owner, const event::Event& ev) {
    event::on(ev,
        [&](const event::PlayerAddedEvent& e) {
            // handle player added
        },
        [&](const event::EntitySelectedEvent& e) {
            if (e.entityId == owner.getID()) {
                // this entity was selected
            }
        }
    );
    // Unhandled alternatives are silently ignored — no need for a catch-all.
}
```

For direct `std::visit` on a non-nested variant (e.g. `HudElementData`), use `overloaded{...}` from `util/Overloaded.hpp`.

## Dispatching events

```cpp
// Scene-internal (LogicEvent):
_entities.handleEvent(event::Event{event::LogicEvent{event::EntitySelectedEvent{id}}});

// World mutation path (WorldEvent emitted by CommandExecutor):
_world.emit(event::WorldEvent{event::PlayerAddedEvent{player}});
```

## WorldEvent — server → scene

Emitted by `World::emit()` after `CommandExecutor` processes a protocol line. `WorldScene::handleEvent` dispatches these to subsystems.

| Event | Fields | Triggered by |
|---|---|---|
| `WorldResizedEvent` | `width, height` | `msz` |
| `TileChangedEvent` | `x, y, resources` | `bct` |
| `PlayerAddedEvent` | `PlayerState player` | `pnw` |
| `PlayerMovedEvent` | `id, x, y, orientation` | `ppo` |
| `PlayerLevelChangedEvent` | `id, level` | `plv` |
| `PlayerInventoryChangedEvent` | `id, inventory` | `pin` |
| `PlayerRemovedEvent` | `id` | `pdi` |
| `PlayerExpelledEvent` | `id` | `pex` |
| `PlayerBroadcastEvent` | `id, message` | `pbc` |
| `IncantationStartEvent` | `x, y, level, playerIds` | `pic` |
| `IncantationEndEvent` | `x, y, success` | `pie` |
| `EggLayingEvent` | `playerId` | `pfk` |
| `ResourceDroppedEvent` | `playerId, resourceIdx` | `pdr` |
| `ResourceCollectedEvent` | `playerId, resourceIdx` | `pgt` |
| `EggAddedEvent` | `EggState egg` | `enw` |
| `EggRemovedEvent` | `id` | `edi` |
| `EggHatchedEvent` | `eggId` | `ebo` |
| `TeamAddedEvent` | `name` | `tna` |
| `TimeUnitChangedEvent` | `timeUnit` | `sgt` |
| `GameEndedEvent` | `winnerTeam` | `seg` |
| `ServerMessageEvent` | `message` | `smg` |
| `ServerUptimeEvent` | `uptimeSeconds` | `suc` |
| `MapLayoutCycleEvent` | — | Key G (InputAction) |
| `TileShadingToggleEvent` | — | Key T (InputAction) |

## LogicEvent — scene-internal

Emitted by scene logic; consumed by entity behaviors. Never originates from the network.

| Event | Fields | Emitted by |
|---|---|---|
| `EntityMoveToEvent` | `entityId, target, duration` | `PlayerTileSystem` |
| `EntityRotateToEvent` | `entityId, targetYaw, duration` | `PlayerTileSystem` |
| `HoverEvent` | `entityId, isHovered` | `PickSystem` / `HudPicker` |
| `SelectEvent` | `entityId, isSelected` | `EntityManager::applySelection` |
| `ClickEvent` | `entityId` | `PickSystem` / `HudPicker` |
| `EntitySelectedEvent` | `entityId` | `SelectableBehavior` |
| `CameraFollowToggleEvent` | — | InputAction F |
| `TeamSelectEvent` | `ids, isSelected` | `TeamDetailPanel` Follow button |

## WindowEvent — OS / input

Emitted by `GraphicsContext::pollAndDispatch()` from raw Raylib events.

| Event | Fields |
|---|---|
| `CloseEvent` | — |
| `ResizeEvent` | `width, height` |
| `MouseButtonEvent` | `button, pressed, x, y` |
| `MouseMoveEvent` | `x, y, dx, dy` |
| `MouseWheelEvent` | `delta` |
| `KeyEvent` | `key, pressed` |

## Adding a new event type

### WorldEvent (from server)

1. Add the struct to `include/event/WorldEvent.hpp`:
   ```cpp
   struct MyNewEvent { uint32_t id; std::string data; };
   ```
2. Add it to the `WorldEvent` variant list.
3. Handle it in `CommandExecutor` — call `_world.emit(event::WorldEvent{MyNewEvent{...}})`.
4. Handle it in `WorldScene::handleEvent` using `event::on`.

### LogicEvent (scene-internal)

1. Add the struct to `include/event/LogicEvent.hpp`.
2. Add it to the `LogicEvent` variant list.
3. Emit it via `_entities.handleEvent(event::Event{event::LogicEvent{MyNewEvent{...}}})`.
4. Handle it in whatever behavior needs it.

No registration, no enum, no factory. Adding to the variant and handling with `event::on` is all that's needed.
