# Architecture

## System overview

```
main.cpp
  └── GameEngine (AppState: Menu | Connecting | InGame)
        ├── CliParser               (argv → port/host/log flags)
        ├── Logger                  (sinks: console + optional file)
        ├── World                   (authoritative game state)
        ├── CommandExecutor         (protocol line → World mutation + emit)
        ├── GuiNetworkManager       (TCP poll loop → CommandParser → CommandExecutor)
        ├── GraphicsContext         (RaylibWindow + RaylibRenderer + RaylibMeshFactory
        │                            + RaylibTextureLoader + RaylibFontLoader)
        ├── RaylibAudioManager      (spatial sound + background music)
        └── IScene
              ├── MenuScene         (main menu / connect / launch screens)
              └── WorldScene
                    ├── EntityManager     (_entities) — 3D world objects
                    ├── HudManager        (_hud)       — 2D HUD entities
                    ├── InputManager      (key/mouse → actions → events)
                    ├── HudPicker         (click dispatch for 2D HUD)
                    ├── PickSystem        (raycast for 3D entity clicks)
                    ├── SceneHudManager   (routes WorldEvents → 12+ HUD panels)
                    ├── PlayerTileSystem  (tile tracking + movement dispatch)
                    ├── AnimationClock    (server ticks → seconds)
                    ├── WorldBuilder      (ground mesh + resource gems)
                    └── CameraController  (orbit / follow / first-person)
```

## Application states

`GameEngine` has three `AppState` values:

| State | Scene | Description |
|---|---|---|
| `Menu` | `MenuScene` | Main menu, connect dialog, launch panel |
| `Connecting` | `MenuScene` | TCP handshake in progress |
| `InGame` | `WorldScene` | Live game rendering |

Transitions: `Menu → Connecting` (user hits connect), `Connecting → InGame` (handshake OK), `InGame → Menu` (disconnect / game over).

## Frame loop

Each frame in `GameEngine::run()`:

1. **Network poll** — `GuiNetworkManager` reads available bytes, pushes complete lines to `CommandParser` → `CommandExecutor` → `World::emit(WorldEvent)` → `WorldScene::handleEvent`.
2. **Input** — `GraphicsContext::pollAndDispatch` collects OS events; `InputManager` maps keys to `InputAction`s and emits `event::Event` to the active scene.
3. **Update** — Active scene `update(world, dt)` ticks every entity behavior (movement interpolation, animations, HUD providers, audio listener).
4. **Render** — 3D pass: `EntityManager` dispatches `RenderEvent` to all behaviors; skybox + grass drawn by renderer. 2D pass: `HudManager` does the same.
5. **Audio** — `RaylibAudioManager::update(dt)` ticks cooldowns, polls music stream.

## Data ownership

```
World          owns → PlayerState, Tile, EggState, team names
WorldScene     owns → Entity objects (EntityManager / HudManager)
               reads → World (const ref) during update
MenuScene      owns → Menu HUD entities
GameEngine     owns → World, both scenes, network, graphics, audio
```

`World` never touches graphics or audio. `WorldScene` never mutates `World` directly — it only calls `_sendLine(cmd)` to request server-side changes.

## Key abstractions

| Abstraction | File | Purpose |
|---|---|---|
| `graphic::Entity` | `include/entity/Entity.hpp` | ID + list of `IBehavior` pointers |
| `behavior::IBehavior` | `include/behavior/IBehavior.hpp` | `onAttach / onUpdate / onEvent` |
| `behavior::hud::IHudProvider` | `include/hud/IHudProvider.hpp` | `getHudElements()` — feeds a panel |
| `event::Event` | `include/event/Event.hpp` | Nested variant of all event categories |
| `event::on(ev, handlers...)` | `include/event/Event.hpp` | Type-safe visitor, recurses nested variants |
| `EntityBuilder` | `src/scene/builder/` | Fluent API — only way to create entities |
| `HudContainerBehavior` | `src/behavior/hud/HudContainerBehavior.hpp` | Renders provider elements; layout/scroll/animation |
| `IMapLayout` | `src/scene/layout/` | Pluggable world geometry (Grid, Torus, Sphere) |
| `audio::IAudioManager` | `include/audio/IAudioManager.hpp` | Spatial sound + music abstraction |

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
| `audio::` | Audio abstraction (`IAudioManager`, `SoundKind`) |

## Entity ID allocation

IDs must not collide across `_entities` and `_hud`. Allocate new ranges away from all existing ones.

| Range | Usage |
|---|---|
| server-assigned player id | Player 3D entity in `_entities` |
| player id + 10000 | Player world-space name tag in `_hud` |
| `1 << 20` (`RESOURCE_BASE_ID`) | Resource gem entities |
| `1 << 21` (`EGG_BASE_ID`) | Egg entities |
| `1 << 23` (`TILE_BASE_ID`) | Ground tile entities |
| `1 << 25` | Wave broadcast effect entities |
| `1 << 26` | Ritual circle effect entities |
| `1 << 27` | Explosion effect entities |
| `1 << 28` (`GROUND_ENTITY_ID`) | Terrain mesh entity |
| 9999 | Resource info HUD |
| 9998 | Player info HUD |
| 9997 | Team chat HUD |
| 9896 | Team detail HUD |
| 9895–9893 (`POPUP_BASE_ID`) | Popup notification slots (3) |
| 9892 | Speed control HUD |
| 9891 | Settings / keybindings HUD |
| 9890 | Inventory HUD |
| 9889 | Clock HUD |
| 9888 | World info HUD |
| 9900–9949 (`LEADERBOARD_BASE_ID`) | Leaderboard entry HUDs (one per team) |
| 9899 (`LEADERBOARD_CTRL_ID`) | Leaderboard control (pagination) HUD |
| 9880 | Main menu HUD (MenuScene) |
| 9881 | Connect panel HUD (MenuScene) |
| 9882 | Launch panel HUD (MenuScene) |

## Adding a new subsystem

1. If it manages entities — give it a reference to `_entities` or `_hud` (passed from `WorldScene`).
2. If it reacts to world events — add a handler in `SceneHudManager` or directly in `WorldScene::handleEvent`.
3. If it sends server commands — receive the `_sendLine` callback; never access `GuiNetworkManager` directly.
4. Wire it up in `WorldScene`'s constructor after `_hudMgr.setup(...)`.
5. If it plays sounds — call `Locator::getAudio()->playAt(kind, pos)`.
