# Architecture

## System overview

```
main.cpp
  └── GameEngine
        ├── CliParser               (argv → port/host/log flags)
        ├── Logger                  (sinks: console + optional file)
        ├── World                   (authoritative game state)
        ├── CommandExecutor         (protocol line → World mutation + emit)
        ├── GuiNetworkManager       (TCP poll loop → CommandParser → CommandExecutor)
        ├── GraphicsContext         (RaylibWindow + RaylibRenderer + RaylibMeshFactory)
        └── IScene → WorldScene
              ├── EntityManager     (_entities) — 3D world objects
              ├── HudManager        (_hud)       — 2D HUD entities
              ├── InputManager      (key/mouse → actions → events)
              ├── HudPicker         (click dispatch for 2D HUD)
              ├── PickSystem        (raycast for 3D entity clicks)
              ├── SceneHudManager   (routes WorldEvents → 6+ HUD panels)
              ├── PlayerTileSystem  (tile tracking + movement dispatch)
              ├── WorldBuilder      (ground mesh + resource gems)
              └── CameraController  (orbit / follow / first-person)
```

## Frame loop

Each frame in `GameEngine::run()`:

1. **Network poll** — `GuiNetworkManager` reads available bytes, pushes complete lines to `CommandParser` → `CommandExecutor` → `World::emit(WorldEvent)` → `WorldScene::handleEvent`.
2. **Input** — `InputManager` maps keys to `InputAction`s, emits `event::Event` to `WorldScene`.
3. **Update** — `WorldScene::update(world, dt)` ticks every entity behavior (movement interpolation, animations, HUD providers).
4. **Render** — 3D pass: `EntityManager` dispatches `RenderEvent` to all behaviors; 2D pass: `HudManager` does the same.

## Data ownership

```
World          owns → PlayerState, Tile, EggState, team names (vectors/maps)
WorldScene     owns → Entity objects (via EntityManager / HudManager)
               reads → World (const ref) during update
```

`World` never touches graphics. `WorldScene` never mutates `World` directly — it only calls `_sendLine(cmd)` to request server-side changes (e.g. `sst <speed>`).

## Key abstractions

| Abstraction | File | Purpose |
|---|---|---|
| `graphic::Entity` | `include/entity/Entity.hpp` | ID + list of `IBehavior` pointers |
| `behavior::IBehavior` | `include/behavior/IBehavior.hpp` | `onAttach / onUpdate / onEvent` |
| `behavior::hud::IHudProvider` | `include/hud/IHudProvider.hpp` | `getHudElements()` — feeds a panel |
| `event::Event` | `include/event/Event.hpp` | Nested variant of all event categories |
| `event::on(ev, handlers...)` | `include/event/Event.hpp` | Type-safe visitor, recurses nested variants |
| `EntityBuilder` | `src/scene/builder/` | Fluent API — only way to create entities |
| `HudContainerBehavior` | `src/behavior/hud/HudContainerBehavior.hpp` | Renders a provider's elements, handles layout/scroll/animation |

## Namespace map

| Namespace | Used for |
|---|---|
| `zappy::` | Application-level (GameEngine, parsers, World, factories) |
| `zappy::net::` | Network protocol types |
| `graphic::` | Abstract interfaces + Entity/Behavior types |
| `graphic::raylib::` | Raylib concrete implementations |
| `behavior::` | Behavior base classes + concrete behaviors |
| `behavior::hud::` | HUD element types + `IHudProvider` |
| `event::` | All event structs + `on()` visitor |

## Entity ID allocation

IDs must not collide across `_entities` and `_hud`. Allocate new ranges away from all existing ones.

| Range | Usage |
|---|---|
| server-assigned player id | Player 3D entity in `_entities` |
| player id + 10000 | Player world-space name tag in `_hud` |
| `1 << 20` | Resource gem entities |
| `1 << 21` | Egg entities |
| `1 << 23` | Ground tile entities |
| `1 << 28` | Terrain mesh entity |
| `1 << 25` | Wave broadcast effects |
| `1 << 26` | Ritual circle effects |
| `1 << 27` | Explosion effects |
| 9999 | Resource info HUD |
| 9998 | Player info HUD |
| 9997 | Team chat HUD |
| 9896 | Team detail HUD |
| 9895–9893 | Popup notification slots |
| 9892 | Speed control HUD |
| 9891 | Settings / keybindings HUD |
| 9890 | Inventory HUD |
| 9900–9949 | Leaderboard entry HUDs (one per team) |

## Adding a new subsystem

1. If it manages entities — give it a reference to `_entities` or `_hud` (passed from `WorldScene`).
2. If it reacts to world events — add a handler in `SceneHudManager` or directly in `WorldScene::handleEvent`.
3. If it sends server commands — receive the `_sendLine` callback; never access `GuiNetworkManager` directly.
4. Wire it up in `WorldScene`'s constructor after `_hudMgr.setup(...)`.
