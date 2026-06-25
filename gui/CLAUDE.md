# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Zappy is an EPITECH Year-End project: a networked multi-player game where AI clients compete on a tile-based map. It has three components:
- **`server/`** (`zappy_server`) — C server managing game state, clients, and the Zappy protocol
- **`gui/`** (`zappy_gui`) — C++20 graphical client using Raylib for 3D visualization
- **`api/`** (`zappy_ai`) — AI client (stub in `api/src/main.c`)

## Build Environment

The GUI requires Raylib. On NixOS / without system-level Raylib, enter the dev shell first:
```bash
cd gui
nix develop   # provides raylib, cmake, X11/Wayland headers
```

### IDE / clangd diagnostics

clangd is **not configured** for this project's include paths. Diagnostics like `'behavior/ABehavior.hpp' file not found` or `undeclared identifier 'graphic'` are false positives. Use `make` (or `make debug`) as the authoritative build check — not IDE error squiggles.

### Build commands (run from `gui/`)
```bash
make          # release build (logging disabled)
make debug    # DEBUG console + TRACE file log → network.log  ← use during dev
make info     # INFO console, no log file
make release  # explicit no-log build
```

### Server
```bash
cd server && make && make tests_run   # build + unit tests
```

### Running
```bash
./zappy_server -p <port> -x <width> -y <height> -c <clients_per_team> -n <team1> [team2 ...] [-f <freq>]
./zappy_gui   -p <port> [-h <host>] [-v <level>] [--log-file <file>] [--log-file-level <level>]
```

## GUI Architecture

### Big picture

```
main.cpp → GameEngine
             ├── IWindowContext  (RaylibWindow)
             ├── IRenderer       (RaylibRenderer)
             ├── IMeshFactory    (RaylibMeshFactory)
             ├── GuiNetworkManager → CommandParser → CommandExecutor → World
             └── IScene (WorldScene)
                   ├── EntityManager    (_entities) — 3D world objects
                   ├── HudManager       (_hud)       — 2D screen-space UI
                   ├── InputManager
                   ├── HudPicker        — click/hover dispatch for HUD
                   ├── PickSystem       — raycasting for 3D world clicks
                   ├── SceneHudManager  — routes WorldEvents to 6 HUD panels
                   ├── PlayerTileSystem — tile tracking + movement dispatch
                   ├── WorldBuilder     — ground mesh + resource gem spawning
                   └── CameraController — follow / first-person logic
```

`World` owns authoritative game state (players, tiles, eggs, teams). `WorldScene` is a thin coordinator (~390 lines) that delegates to subsystems; it never owns HUD or tile logic directly.

### Scene source layout

```
src/scene/
├── camera/      OrbitCamera, CameraController
├── world/       WorldBuilder, PlayerTileSystem, AnimationClock
├── hud/
│   ├── chat/        ChatPanel        (TeamChatStore, chat bubbles)
│   ├── inventory/   InventoryPanel   (selected player resource slots)
│   ├── leaderboard/ LeaderboardPanel (TeamLeaderboardStore, lb entries)
│   │                TeamDetailPanel  (team detail view, per-player Follow button)
│   ├── player/      PlayerInfoPanel  (selected player info)
│   ├── popup/       PopupPanel       (3 toast slots, timers)
│   ├── resource/    ResourceInfoPanel (tile resource info on click)
│   ├── settings/    SettingsPanel    (keybindings remapper, ID 9891)
│   └── speed/       SpeedPanel       (speed slider)
│   SceneHudManager.hpp/.cpp          (thin coordinator, ≤12 public methods)
├── builder/     EntityBuilder + sub-builders (transform/drawable/hud/…)
├── factory/     PlayerEntityFactory, EggEntityFactory, ResourceEntityFactory, MapGroundFactory
├── layout/      GridLayout, TorusLayout, SphereLayout
├── grass/       GrassBuilder
├── Scene.hpp/.cpp
└── WorldScene.hpp/.cpp
```

### Entity / Behavior system

Every visible or interactive object is a `graphic::Entity` (identified by `EntityID`) holding a list of `shared_ptr<IBehavior>`. Behaviors are the only place where logic lives.

Key hierarchy:
- `IBehavior` → `ABehavior` (no-op `onAttach`/`onEvent`) → concrete behaviors
- `IDrawable` → `ADrawable` (implements `onEvent` to call `draw()` on `RenderEvent`) → mesh/text drawables
- `HudContainerBehavior : ADrawable` — renders a provider's elements, handles scroll and button clicks

Behaviors are attached and retrieved with templates:
```cpp
entity.addBehavior<MyBehavior>(args...);
auto b = entity.getBehavior<MyBehavior>(); // returns shared_ptr<MyBehavior>
```

### EntityBuilder — fluent API for creating entities

Never construct entities manually. Use `EntityBuilder`:
```cpp
EntityBuilder(entityManager, id, "type_tag")
    .transform().scale(1.0f)
    .drawable().model(renderer, modelHandle, color, rotation)
    .interaction().onClick([](graphic::Entity& e){ ... })
    .interaction().selectableOutline()
    .hud().container(providerPtr)
    .hud().background(true, fillColor, borderColor)
    .hud().anchor(graphic::Anchor::TopRight)
    .hud().hidden()           // start hidden (AnimState::Hidden, no animation)
    .build();
```
`build()` returns `EntityPtr` so you can retrieve behaviors immediately after.

### HUD system

HUD entities live in `_hud` (`HudManager`, a typedef of `EntityManager`). They are rendered in 2D after the 3D pass. Screen-space HUDs use anchor positioning; world-space tags use `isWorldSpaceTag(true)` and get positioned by `TagBehavior`.

**Provider pattern** — the only way to feed content into a `HudContainerBehavior`:
```cpp
class MyProvider : public behavior::hud::IHudProvider {
public:
    std::vector<behavior::hud::HudElement> getHudElements() const override { ... }
};
```

`getHudElements()` is called every frame. Available element types: `TextData`, `BarData`, `RectData`, `ButtonData` (clickable, hover-tinted), `ChatBubbleData` (left/right chat bubbles with auto-scroll support), `ImageData` (texture handle + dimensions), `SliderData` (draggable value slider — `onChange` fires every drag step for visual feedback, `onRelease` fires once on mouse-up to commit), `SlotData` (icon + label tile for inventory grids — texture handle, label string, slotSize/imageSize in design-pixels).

Layout types: `Vertical` (default, center-aligned), `Horizontal`, `MediaObject` (element[0] in left column, elements[1..N] stacked in right column — used for leaderboard rows with avatar image).

`HudContainerBehavior` supports scrollable containers (`.setScrollable(true)`, `.scrollToBottom()`). Button clicks are detected internally from `MouseButtonEvent`; no separate entity is needed per button.

Full `EntityBuilder` HUD chain:
```cpp
.hud().container(providerPtr)          // attach provider (required first)
.hud().layout(Type::Vertical, 8.f)    // layout type + padding
.hud().anchor(graphic::Anchor::TopRight)
.hud().anchorOffset({10.f, 175.f})    // raw design-pixel offset; auto-scaled at runtime
.hud().background(true, fillColor, borderColor)
.hud().boxSize({300.f, 400.f})        // fixed size (also auto-scaled at runtime)
.hud().autoSize()                     // revert to auto (default)
.hud().title("Panel Title", 14.f)     // adds a title bar above content
.hud().isWorldSpaceTag(true)          // drives position from TagBehavior, not anchor
.hud().hidden()                       // start hidden (AnimState::Hidden, no animation)
```

**UI scaling** — `HudContainerBehavior` automatically scales all element sizes, padding, margins, `anchorOffset`, and `fixedSize` by `_uiScale = viewport.x / 1280.f` (clamped to ≥ 0.5). Author all pixel values (fontSize, width, height, offsets) at the 1280×720 reference resolution; the system scales them for other window sizes. `overloaded{}` (`util/Overloaded.hpp`) is used for `std::visit` on `HudElementData` — same pattern as `event::on`.

**Visibility & animation** — `HudContainerBehavior` has an internal `AnimState` (`Hidden`, `FadingIn`, `Visible`, `FadingOut`) with slide+fade transitions. Use `setVisible(bool, float duration=-1)` at runtime; use `.hud().hidden()` in the builder for containers that start hidden. Never call `setVisible` every frame with a duration arg — re-entry is guarded but `_animDuration` must not be mutated by instant/no-op paths. World-space tags (driven by `TagBehavior::onUpdate` each frame) rely on the re-entry guard to let the animation complete.

`HudPicker` intercepts clicks on HUD entity bounding boxes before the 3D `PickSystem` sees them. Non-interactable containers (`isInteractable() == false`, i.e. `Hidden` or `FadingOut`) are skipped by `HudPicker`.

### Event system

`event::Event` is a nested variant: `variant<WindowEvent, RenderEvent, WorldEvent, LogicEvent>`, where each of those is itself a `variant<...>`. The helper `event::on(ev, handlers...)` recurses into nested variants automatically:
```cpp
event::on(ev,
    [&](const event::PlayerBroadcastEvent& e) { ... },
    [&](const event::EntitySelectedEvent&  e) { ... }
);
```
Unhandled variant alternatives are silently ignored. For direct `std::visit` on a non-nested variant (e.g. `HudElementData`), use `overloaded{...}` from `util/Overloaded.hpp` instead of a generic lambda with `if constexpr`.

Never use `std::get_if<event::WorldEvent>` + `std::get_if<inner_event>` manually — `event::on` unwraps nesting automatically. If early exit is needed after handling a specific event, set a local `bool handled` flag inside the lambda and check it after the call.

**WorldEvent** (from network → World → scene): `PlayerAddedEvent`, `PlayerMovedEvent`, `PlayerBroadcastEvent`, `TileChangedEvent`, etc.  
**LogicEvent** (scene-internal): `EntityMoveToEvent`, `EntityRotateToEvent`, `ClickEvent`, `SelectEvent`, `EntitySelectedEvent`.

`SelectableBehavior` emits `EntitySelectedEvent` to the scene when its entity is clicked. `EntityManager::handleEvent` intercepts `EntitySelectedEvent` to call `applySelection()`, which propagates `SelectEvent` to all entities.

### Entity ID allocation

IDs must not collide across managers. Convention:

| Range | Usage |
|---|---|
| `playerId` (server-assigned) | Player 3D entities in `_entities` |
| `playerId + 10000` | Player world-space tag HUD entity in `_hud` |
| `1 << 20` (`RESOURCE_BASE_ID`) | Resource gem entities |
| `1 << 21` (`EGG_BASE_ID`) | Egg entities |
| `1 << 23` (`TILE_BASE_ID`) | Ground tile entities |
| `1 << 28` (`GROUND_ENTITY_ID`) | Terrain mesh entity |
| `9999` | Resource info HUD (Scene) |
| `9998` | Selected-player info HUD (WorldScene) |
| `9997` | Team chat HUD (WorldScene) |
| `9896` | Team detail HUD (WorldScene) |
| `9893–9895` | Notification popup HUD slots (`POPUP_BASE_ID`, 3 slots) |
| `9892` | Speed control HUD (WorldScene) |
| `9891` | Settings/keybindings HUD (WorldScene) |
| `9890` | Inventory HUD (WorldScene) |
| `9900–9949` | Leaderboard entry HUDs, one per team (`LEADERBOARD_BASE_ID`) |

### Leaderboard system

`TeamLeaderboardStore` (`src/world/TeamLeaderboardStore.hpp`) tracks players per team and their levels. `getRankedTeams()` returns teams sorted by max player level (descending). `LeaderboardPanel` (`src/scene/hud/leaderboard/`) owns the store and calls `recomputeLeaderboard()` on every player add/remove/level-change, repositioning HUD entries vertically.

Each team gets one `leaderboard_entry` HUD entity. `spawnLeaderboardEntry` is idempotent — checks `_lbEntityIds.count(team)` before spawning. Entries use `LayoutEngine::Type::MediaObject` layout (image left column, text+button right column).

`TeamDetailPanel` (`src/scene/hud/leaderboard/`) — opened via the "Details" `ButtonData`. Toggle logic uses `isFullyVisible()` (animState == Visible), not `isVisible()` (_isVisible flag), because `setVisible(true)` sets `_isVisible` immediately while animation is still `FadingIn`; using `isVisible()` for toggle re-closes the panel on the same click.

### Sending commands to the server

`WorldScene` holds a `std::function<void(std::string)> _sendLine` callback set by `GameEngine` after construction (`setSendLine`). Use it to send protocol commands from the scene, e.g. `_sendLine("sst 10")`. Never call `GuiNetworkManager` directly from scene or provider code — always go through this callback.

### Default keybindings

| Key | Action |
|---|---|
| W / ↑, S / ↓, A / ←, D / → | Camera pan |
| Space | Toggle POV (orbit center ↔ free FPS) |
| F | Follow toggle (single: follow/unfollow; double-tap: first-person) |
| G | Cycle map layout (Grid ↔ Torus) |
| T | Toggle tile shading |
| Tab | Show/hide leaderboard |
| O | Open/close settings panel |

### Inventory & Settings panels

`InventoryPanel` (`src/scene/hud/inventory/`) shows the selected player's resources as `SlotData` icon tiles. It tracks `_selectedPlayerId` internally; call `setSelectedPlayer(id)` on selection and `onInventoryChanged(id, inv)` on `pin` events — it silently ignores updates for non-selected players.

`SettingsPanel` (`src/scene/hud/settings/`) wraps `KeybindingsSection` — a live remapper that intercepts the next key press to rebind an action. It receives an `InputManager&` reference at setup so it can call `rebindAction` directly. Toggle via `Tab`-adjacent binding wired through `InputAction::TOGGLE_SETTINGS`.

### Panel callback wiring pattern

Panels expose `setOn*Click(std::function<...>)` methods; `WorldScene` binds them in its constructor after `_hudMgr.setup(...)`. Example from the current codebase:

```cpp
// Wire follow-click: select player and start camera follow from TeamDetailPanel
_hudMgr.teamDetail().setOnFollowClick([this](uint32_t id) {
    handleEvent(event::Event{event::LogicEvent{event::EntitySelectedEvent{id}}});
    if (!_camera.isFollowing())
        _camCtrl.onFollowToggle(_selectedPlayerId, _entities, _hud);
});
```

`_camera` is `OrbitCamera` (inherited from `Scene`). `_camCtrl` is `CameraController` (wraps `_camera`). If `_camera.isFollowing()` is already true, `onEntitySelected` automatically switches the follow target — no need to call `onFollowToggle` again.

### Team chat / broadcast

`TeamChatStore` (`src/world/TeamChatStore.hpp`) accumulates `pbc` broadcast messages per team. `ChatPanel` (`src/scene/hud/chat/`) owns the store and is fed by `SceneHudManager` on `PlayerAddedEvent` (team mapping) and `PlayerBroadcastEvent` (messages). `TeamChatProvider` reads it to render chat bubbles; `PlayerInfoPanel` renders the selected-player panel with the "Team Chat" button that opens the chat HUD.

### Network → World flow

`GuiNetworkManager` streams raw lines → `CommandParser` tokenizes → `CommandExecutor` calls typed handlers (one per protocol command: `msz`, `bct`, `pnw`, `ppo`, `pbc`, etc.) → `World::emit(WorldEvent)` → `WorldScene::handleEvent`.

### Locator

Global service locator for cross-cutting dependencies:
```cpp
Locator::provide(logger);  Locator::getLogger();
Locator::provide(scene);   Locator::getScene();
Locator::provide(renderer); Locator::getRenderer();
```

### Logging

Declare as a class member: `ContextLogger _log{"ClassName"};`  
Use: `_log.debug("x = ", x);` — variadic, no format string needed.  
Levels: `TRACE < DEBUG < INFO < WARNING < ERROR < NONE`. Compile-time gated; runtime override via `-v`.

### Namespaces

| Namespace | Used for |
|---|---|
| `zappy::` | Application-level (GameEngine, managers, parsers, World, factories) |
| `zappy::net::` | Network protocol types (`Message`, `MessageKind`) |
| `graphic::` | Abstract interfaces and Entity/Behavior types |
| `graphic::raylib::` | Raylib concrete implementations |
| `behavior::` | Behavior base classes and concrete behaviors |
| `behavior::hud::` | HUD element types and `IHudProvider` |
| `event::` | All event types and the `on()` visitor helper |

### ExplosionBehavior / particle budget

Total live sparks = `NUM_FW × N_ROCKETS × N_SPARKS`. Keep under ~150 to avoid frame drops. Render-path hot loops must not call `sqrt` or recompute per-frame values — move those to `onUpdate`. Current tuned values: `NUM_FW=4`, `N_ROCKETS=3`, `N_SPARKS=12`.

## PR Format

```
## FIXES
Issue #

## Description
### How
### Testing
1. Execution:
2. Validation:
### Notes
- Next:
- Technical Details:
- Refactoring:
```
