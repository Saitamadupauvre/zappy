# Zappy GUI

> **C++20** graphical client for the Zappy networked multiplayer game.
> Connects to `zappy_server` and renders the live game world in real-time 3D.

Built with **[Raylib](https://www.raylib.com/)** on a custom entity-behavior engine, with a fully animated 2D HUD layer, shader effects, and a reactive event-driven architecture.

---

## Features

- **3D world rendering** — tile-based map with grid and torus layout modes, skybox, animated grass
- **Live player tracking** — smooth movement interpolation, orientation, level-up animations, follow/first-person camera
- **Shader effects** — crystal gems, incantation ritual circles, broadcast wave rings, explosion particles
- **Full HUD** — leaderboard, team chat, inventory, player info, speed control, keybinding remapper, popup notifications
- **Real-time network sync** — non-blocking TCP poll loop, full Zappy GUI protocol support
- **Configurable logging** — multi-level, multi-sink (console + file), compile-time gated

---

## Build

**Requirements:** C++20 compiler, CMake, Raylib.

```bash
make          # release — no logging output
make debug    # DEBUG level console + TRACE file log → network.log
make info     # INFO level console, no file
make release  # explicit alias for release
```

> On **NixOS** or systems without a system-level Raylib, enter the dev shell first:
> ```bash
> nix develop
> ```

---

## Run

```bash
./zappy_gui -p <port> [options]
```

| Flag | Default | Description |
|:--|:--|:--|
| `-p PORT` | *(required)* | Server port |
| `-h HOST` | `localhost` | Server hostname |
| `-v LEVEL` | `WARNING` | Console log level |
| `--log-file FILE` | — | Write logs to file |
| `--log-file-level LEVEL` | `TRACE` | File log verbosity |

Log levels: `TRACE` `DEBUG` `INFO` `WARNING` `ERROR` `NONE`

**Example:**
```bash
./zappy_gui -p 4242 -h 127.0.0.1 -v DEBUG --log-file network.log
```

---

## Controls

| Input | Action |
|:--|:--|
| `Z` / `↑` &nbsp; `S` / `↓` &nbsp; `Q` / `←` &nbsp; `D` / `→` | Pan camera |
| Mouse drag (right button) | Orbit camera |
| Scroll wheel | Zoom in / out |
| `Space` | Toggle orbit ↔ free-FPS camera |
| `F` | Follow selected player · double-tap: first-person view |
| `G` | Cycle map layout (Grid ↔ Torus) |
| `T` | Toggle tile shading |
| `Tab` | Show / hide leaderboard |
| `O` | Open / close settings panel |
| Click on player / resource | Select entity |

> Keybindings are fully remappable at runtime via the **Settings panel** (`O`).

---

## Architecture overview

```
main.cpp
 └─ GameEngine
     ├─ CliParser           argv → port / host / log config
     ├─ Logger              console sink + optional file sink
     ├─ World               authoritative game state (players, tiles, eggs, teams)
     ├─ CommandExecutor     protocol line → World mutation → WorldEvent
     ├─ GuiNetworkManager   TCP poll loop → CommandParser → CommandExecutor
     ├─ GraphicsContext     RaylibWindow · RaylibRenderer · RaylibMeshFactory
     └─ WorldScene
         ├─ EntityManager   3D world entities (players, resources, eggs, terrain)
         ├─ HudManager      2D screen-space HUD entities
         ├─ InputManager    key/mouse → InputAction → scene events
         ├─ CameraController   orbit / follow / first-person transitions
         ├─ SceneHudManager    routes WorldEvents to HUD panels
         ├─ PlayerTileSystem   tile tracking + movement interpolation dispatch
         └─ WorldBuilder       ground mesh + resource gem spawning
```

**Data flow:** `TCP bytes → CommandParser → CommandExecutor → World::emit(WorldEvent) → WorldScene::handleEvent → entity behaviors`

The **Entity/Behavior** system is the rendering and logic core — every visible object is an `Entity` holding a list of `IBehavior` implementations that react to update ticks and events.

---

## HUD panels

| Panel | Toggle | Description |
|:--|:--|:--|
| Player info | click player | Name, team, level, orientation |
| Inventory | click player | Resource slots with icons |
| Team chat | click player → Chat | Per-team broadcast messages |
| Leaderboard | `Tab` | Teams ranked by max player level |
| Team detail | click team → Details | Per-player list + Follow button |
| Speed control | always visible | Game speed slider (sends `sst`) |
| Settings | `O` | Live keybinding remapper |
| Resource info | click tile | Tile resource counts |
| Notifications | automatic | Toast popups for game events |

---

## Visual effects

| Effect | Shader | Trigger |
|:--|:--|:--|
| Crystal gems | `crystal.vs/.fs` | Resources on tiles |
| Animated grass | `grass.vs/.fs` | Ground tiles |
| Wave ring | `wave.vs/.fs` | Player broadcast (`pbc`) |
| Ritual circle | `ritual.vs/.fs` | Incantation start (`pic`) |
| Explosion burst | `explosion.vs/.fs` | Incantation end (`pie`) |
| Selection outline | `outline.vs/.fs` | Entity selected |
| Skybox | `skybox.vs/.fs` | Always |

---

## Source layout

```
gui/
├── assets/
│   ├── model/              player_lv1.glb  apple.glb  crystal.glb
│   ├── shaders/            GLSL shader pairs (.vs / .fs)
│   └── images/             Textures
│
├── include/                Public headers (interfaces + types)
│   ├── behavior/           IBehavior, ABehavior, IDrawable, ADrawable
│   ├── entity/             Entity.hpp
│   ├── event/              Event.hpp, WorldEvent, LogicEvent, RenderEvent
│   ├── graphic/            IRenderer, IMeshFactory, IWindowContext, Types
│   ├── hud/                HudElements.hpp, IHudProvider.hpp
│   ├── logger/             ContextLogger, ISink, LogLevel
│   ├── network/            GuiProtocol.hpp (MessageKind enum)
│   ├── scene/              IScene, TileMap, IMapLayout
│   └── world/              WorldTypes.hpp (PlayerState, EggState, Tile)
│
├── src/
│   ├── behavior/
│   │   ├── animation/      PlayerAnimationBehavior
│   │   ├── clickable/      ClickableBehavior
│   │   ├── drawable/       ModelDrawable, MeshDrawable, TextDrawable, HudContainer
│   │   ├── egg/            EggBehavior
│   │   ├── hoverable/      HoverableBehavior
│   │   ├── hud/            HudContainerBehavior, LayoutEngine, all Providers
│   │   ├── incantation/    RitualCircle, Explosion, IncantationTile
│   │   ├── movement/       MovementBehavior, RotationBehavior
│   │   ├── outline/        OutlineBehavior
│   │   ├── pick/           PickSystem
│   │   ├── player/         PlayerBehavior, PlayerMovement, BroadcastBehavior
│   │   ├── resource/       ResourceBehavior
│   │   ├── selectable/     SelectableBehavior
│   │   ├── tag/            TagBehavior (world-space HUD anchor)
│   │   ├── transform/      TransformBehavior
│   │   └── wave/           WaveBroadcastBehavior
│   │
│   ├── core/
│   │   ├── executor/       CommandExecutor (protocol → WorldEvent)
│   │   ├── manager/        EntityManager, HudManager, InputManager
│   │   └── GameEngine.cpp
│   │
│   ├── graphics/           GraphicsContext + Raylib implementations
│   │   └── raylib/         RaylibRenderer, Window, MeshFactory, Shaders, Textures
│   │
│   ├── locator/            Locator.cpp (service locator: logger, renderer, scene)
│   ├── logger/             Logger.cpp (multi-sink, compile-time level gating)
│   │
│   ├── network/
│   │   ├── client/         GuiConnection (buffered TCP read/write)
│   │   ├── poll/           PollManager (non-blocking fd poll)
│   │   ├── socket/         Socket.cpp
│   │   └── GuiNetworkManager.cpp
│   │
│   ├── parser/
│   │   ├── cli/            CliParser (argv flags)
│   │   ├── command/        ICommand implementations (one per protocol message)
│   │   ├── CommandParser/  raw line → net::Message{kind, args}
│   │   ├── flag/           StateFlag, ValueFlag
│   │   └── Resources/      Resources struct parser
│   │
│   ├── scene/
│   │   ├── camera/         OrbitCamera, CameraController
│   │   ├── factory/        PlayerEntityFactory, EggEntityFactory,
│   │   │                   ResourceEntityFactory, MapGroundFactory, TileEntityFactory
│   │   ├── grass/          GrassBuilder
│   │   ├── hud/            ChatPanel, InventoryPanel, LeaderboardPanel,
│   │   │                   PlayerInfoPanel, PopupPanel, ResourceInfoPanel,
│   │   │                   SettingsPanel, SpeedPanel, TeamDetailPanel, ClockPanel
│   │   ├── layout/         GridLayout, TorusLayout, SphereLayout
│   │   ├── world/          WorldBuilder, PlayerTileSystem, AnimationClock
│   │   ├── SceneHudManager.cpp
│   │   └── WorldScene.cpp
│   │
│   └── world/
│       ├── World.cpp
│       ├── TeamChatStore.hpp
│       └── TeamLeaderboardStore.hpp
│
├── docs/                   Developer documentation (see below)
└── _doc/                   Original subject + protocol specification
```

---

## Developer documentation

| Document | What it covers |
|:--|:--|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | System overview, frame loop, data ownership, ID allocation |
| [docs/CLASS_DIAGRAM.md](docs/CLASS_DIAGRAM.md) | Mermaid class diagrams for all subsystems |
| [docs/ENTITY_BEHAVIOR.md](docs/ENTITY_BEHAVIOR.md) | How to create entities, write behaviors, use EntityBuilder |
| [docs/HUD.md](docs/HUD.md) | How to add HUD panels, providers, and UI element types |
| [docs/EVENTS.md](docs/EVENTS.md) | All event types, how to handle and add new ones |
| [docs/NETWORK.md](docs/NETWORK.md) | Protocol reference, adding new server commands |
| [docs/LOGGER.md](docs/LOGGER.md) | Logging API and sink configuration |
