# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Zappy is an EPITECH Year-End project: a networked multi-player game where AI clients compete on a tile-based map. It has three components:
- **`server/`** (`zappy_server`) — C server managing game state, clients, and the Zappy protocol
- **`gui/`** (`zappy_gui`) — C++20 graphical client using Raylib for 3D visualization
- **`api/`** (`zappy_ai`) — AI client (stub in `api/src/main.c`)

## Build Commands

### Full project
```bash
make          # builds all three binaries
make re       # clean rebuild
```

### GUI only (most active component)
```bash
cd gui
make          # release/silent build
make debug    # DEBUG console + TRACE file log → network.log
make info     # INFO console, no log file
make release  # all logging disabled
```

### Server only
```bash
cd server
make
make tests_run   # build and run unit tests
```

### Server usage
```
./zappy_server -p <port> -x <width> -y <height> -c <clients_per_team> -n <team1> [team2 ...] [-f <frequency>]
```

### GUI usage
```
./zappy_gui -p <port> [-h <host>] [-v <level>] [--log-file <file>] [--log-file-level <level>]
```

## Architecture

### GUI — `gui/src/` and `gui/include/`

The GUI is built around a **component/behavior entity system** sitting on top of an abstracted graphics layer.

**Entry point**: `main.cpp` → `zappy::GameEngine` (owns all subsystems and runs the main loop).

**Core subsystems** (`gui/src/core/`):
- `GameEngine` — orchestrates the window, input, network, and command processing
- `EntityManager` — owns all `graphic::Entity` instances; calls `update()` and `handleEvent()` each frame
- `InputManager` — maps hardware events (`graphic::KeyCode`) to abstract `InputAction` enums; supports listener callbacks and polling

**Entity/Behavior system** (`gui/include/entity/`, `gui/include/behavior/`):
- `graphic::Entity` — identified by `EntityID` (uint), holds a `vector<shared_ptr<IBehavior>>`
- `IBehavior` → `ABehavior` (base class with no-op defaults for `onAttach`/`onEvent`) → concrete behaviors
- `IDrawable` → `ADrawable` (combines `IBehavior` + `IDrawable`) → `MeshDrawableBehavior`, `TextDrawableBehavior`
- Behaviors are added/retrieved via templated `entity.addBehavior<T>()` / `entity.getBehavior<T>()`

**Graphics abstraction** (`gui/include/graphic/`):
- `IWindowContext` — window lifecycle, frame delimiters, event polling
- `IRenderer` — 3D/2D draw calls, mesh/texture/font resource handles
- `IMeshFactory` — procedural mesh generation (cube, sphere, torus)
- Raylib implementations: `RaylibWindow`, `RaylibRenderer`, `RaylibMeshFactory` in `gui/src/graphics/raylib/`

**Network** (`gui/src/network/`):
- `GuiNetworkManager` — TCP client that handles the Zappy GUI handshake (`WELCOME` → server info → `GRAPHIC`) and streams server messages
- `CommandParser` — tokenizes raw lines into `zappy::net::Message{kind, command, args, raw}`
- `CommandExecutor` — dispatches `net::Message` by `MessageKind` to typed handler methods (one per Zappy protocol command: `msz`, `bct`, `pnw`, `ppo`, etc.)
- Protocol message types are defined in `gui/include/network/GuiProtocol.hpp`

**Service locator** (`gui/src/locator/`): `Locator::provide(Logger*)` / `Locator::getLogger()` — global logger access without passing it everywhere.

**Logging** (`gui/src/logger/`, `gui/include/logger/`):
- `Logger` — multi-sink, compile-time level gating. Sinks: `ConsoleSink`, `FileSink`
- `ContextLogger` — thin wrapper used as a member `ContextLogger _log{"ClassName"}` to auto-tag log origins
- Log levels: `TRACE < DEBUG < INFO < WARNING < ERROR < NONE`
- Compile-time defaults set via CMake: `GUI_CONSOLE_LOG_LEVEL`, `GUI_FILE_LOG_LEVEL`, `GUI_LOG_FILE_PATH`

### Server — `server/`

C project. Source under `server/src/`:
- `main.c` → `handle_pre_serv_proc()` (in `components/`)
- `components/parsing/` — CLI argument parsing with `getopt` (`-p`, `-x`, `-y`, `-c`, `-f`, `-n`)
- `errors/` — allocation and parsing error helpers
- Headers in `server/include/`

### Namespaces

| Namespace | Used for |
|---|---|
| `zappy::` | GUI application-level code (GameEngine, managers, parsers) |
| `zappy::net::` | Network protocol types (`Message`, `MessageKind`) |
| `graphic::` | Abstract graphics interfaces and Entity/Behavior types |
| `graphic::raylib::` | Raylib concrete implementations |
| `behavior::` | Behavior base classes (`IBehavior`, `ABehavior`, `IDrawable`, `ADrawable`) |

## PR Format

PRs should follow the template in `docs/pr_format.md`:
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
