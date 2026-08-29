# Zappy

> A multi-component networked game where autonomous AI agents compete to elevate their Trantorian civilization to level 8.

> This is a school project completed as part of my Bachelor's coursework in Computer Science at Epitech.

## What I did

Group project (multiple teammates). I worked on the networked server/protocol side of the system, alongside teammates who built the 3D GUI client and the Python AI agents (including a reinforcement-learning strategy under `ai/rl/`).

## Skills

- Network protocol design over TCP for a real-time multiplayer game
- C systems programming (authoritative game-server state, rule enforcement)
- Multi-language, multi-process system integration (C server, C++/Raylib GUI, Python AI)
- Team collaboration across independently-built components communicating over a shared protocol
- Reinforcement learning fundamentals (exposure via the AI component's RL strategy training)

Zappy is an **EPITECH Year-2 end-of-year project** built in three independent components that communicate over TCP:

| Component | Language | Role |
|:--|:--|:--|
| `zappy_server` | C (C99) | Authoritative game engine — owns world state, enforces rules |
| `zappy_gui` | C++20 + Raylib | Real-time 3D spectator client |
| `zappy_ai` | Python 3.10+ | Autonomous AI player agent |

---

## Table of Contents

1. [Game Overview](#game-overview)
2. [Quick Start](#quick-start)
3. [Build](#build)
   - [Dependencies](#dependencies)
   - [Build All](#build-all)
   - [Build Individually](#build-individually)
4. [Usage](#usage)
   - [Server](#server)
   - [GUI](#gui)
   - [AI](#ai)
5. [Project Structure](#project-structure)
6. [Documentation](#documentation)
7. [Contributing](#contributing)

---

## Game Overview

The map is a toroidal grid populated with **6 resources**: food, linemate, deraumere, sibur, mendiane, phiras, and thystame. Each AI player (a *Trantorian*) starts at level 1 and must gather resources and perform **incantation rituals** with teammates to reach level 8. The first team to get **6 players to level 8** wins.

```
┌─────────────────────────────────────────────────────┐
│                    zappy_server                     │
│        (world state · rules · win condition)        │
│                                                     │
│   AI protocol (TCP)        GUI protocol (TCP)       │
└────────┬──────────────────────────┬─────────────────┘
         │                          │
    ┌────▼────┐                ┌────▼────┐
    │zappy_ai │                │zappy_gui│
    │(Python) │                │ (C++20) │
    └─────────┘                └─────────┘
```

The server speaks two distinct line-based TCP protocols — one for AI clients, one for GUI spectators. Multiple AI clients and GUI instances can connect simultaneously.

---

## Quick Start

```bash
# 1 — build everything
make

# 2 — start the server (terminal 1)
./zappy_server -p 4242 -x 20 -y 20 -n MY_TEAM -c 6 -f 100

# 3 — open the GUI (terminal 2)
./zappy_gui -p 4242 -h localhost

# 4 — launch the AI (terminal 3)
./zappy_ai -p 4242 -n MY_TEAM -h localhost
```

The AI is fully autonomous. One instance self-elects as **Queen**; the rest become **Followers** via encrypted broadcast coordination.

---

## Build

### Dependencies

| Component | Requirement |
|:--|:--|
| Server | `gcc`, `make`, POSIX (`poll(2)`) |
| GUI | `g++` (C++20), `cmake`, [Raylib](https://www.raylib.com/) |
| AI | Python 3.10+, [`uv`](https://github.com/astral-sh/uv) |

Install `uv` (AI dependency manager):
```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

### Build All

```bash
make        # builds all three binaries at the project root
make re     # full rebuild
make clean  # remove build artifacts
make fclean # clean + remove binaries
```

### Build Individually

**Server:**
```bash
cd server
make              # builds server/build/bin/zappy_server
make re
make clean / fclean
make tests_run    # build + run unit tests
```

**GUI:**
```bash
cd gui
make          # release build (logging off)
make debug    # DEBUG console + TRACE file log → network.log
make info     # INFO console, no file
make release  # explicit release alias
```

**AI:**
```bash
cd ai
make          # wraps uv sync, produces zappy_ai wrapper script
# or manually:
uv sync       # install deps into .venv
```

---

## Usage

### Server

```
./zappy_server -p port -x width -y height -n name1 [name2 ...] -c clientsNb -f freq
./zappy_server --help
```

| Flag | Meaning | Required |
|:--|:--|:--|
| `-p PORT` | TCP listening port (1–65535) | yes |
| `-x WIDTH` | Map width in tiles | yes |
| `-y HEIGHT` | Map height in tiles | yes |
| `-n NAME…` | One or more team names | yes |
| `-c N` | Max clients per team | yes |
| `-f FREQ` | Server ticks per second | yes |

Example:
```bash
./zappy_server -p 4242 -x 20 -y 20 -n TeamA TeamB -c 6 -f 100
```

### GUI

```
./zappy_gui -p PORT [-h HOST] [-v LEVEL] [--log-file FILE] [--log-file-level LEVEL]
```

| Flag | Default | Description |
|:--|:--|:--|
| `-p PORT` | *(required)* | Server port |
| `-h HOST` | `localhost` | Server hostname |
| `-v LEVEL` | `WARNING` | Console log level |
| `--log-file FILE` | — | Write logs to file |
| `--log-file-level LEVEL` | `TRACE` | File log verbosity |

Log levels: `TRACE` `DEBUG` `INFO` `WARNING` `ERROR` `NONE`

#### GUI Controls

| Input | Action |
|:--|:--|
| `W/↑` `S/↓` `A/←` `D/→` | Pan camera |
| Mouse drag | Orbit (yaw / pitch) |
| Scroll wheel | Zoom |
| `Space` | Toggle orbit ↔ free-FPS camera |
| `F` | Follow player · double-tap: first-person |
| `G` | Cycle map layout (Grid → Torus → Sphere) |
| `T` | Toggle tile shading |
| `Tab` | Show / hide leaderboard |
| `O` | Settings panel (keybinding remapper) |
| Click entity | Select / inspect |

### AI

```
./zappy_ai -p PORT -n TEAM [-h HOST] [--no-encrypt] [-d]
```

| Flag | Description |
|:--|:--|
| `-p PORT` | Server port (required) |
| `-n TEAM` | Team name (required) |
| `-h HOST` | Server hostname (default: localhost) |
| `--no-encrypt` | Disable broadcast encryption (debug) |
| `-d / --debug` | Verbose debug output |

Debug logs (CSV) are written to `ai/logs/run_<timestamp>/` on each run.

Run AI tests:
```bash
uv run pytest            # all tests
uv run pytest -v         # verbose
uv run pytest --cov=ai   # with coverage
```

---

## Project Structure

```
RealZappy/
├── zappy_server        ← compiled server binary (after make)
├── zappy_gui           ← compiled GUI binary (after make)
├── zappy_ai            ← compiled AI wrapper (after make)
├── Makefile            ← root orchestrator
│
├── server/             ← C server source
│   ├── src/
│   │   ├── components/ ← game logic (players, map, resources, incantation…)
│   │   ├── errors/     ← error handling
│   │   └── lib/        ← internal utilities
│   ├── include/
│   └── tests/          ← unit tests (criterion)
│
├── gui/                ← C++20 GUI source
│   ├── src/            ← entity-behavior engine, network, HUD, scenes
│   ├── include/        ← public interfaces and types
│   ├── assets/         ← models (.glb), shaders (GLSL), audio, textures
│   └── docs/           ← developer documentation (see below)
│
├── ai/                 ← Python AI source
│   ├── src/            ← FSM, queen/follower logic, network client
│   ├── tests/          ← pytest unit tests
│   └── logs/           ← per-run CSV logs
│
├── docs/               ← cross-component documentation
│   ├── server.md       ← server protocol + architecture reference
│   └── ai_documentation.md ← full AI strategy + architecture guide
│
├── faceit/             ← FACEIT integration
└── scripts/            ← helper scripts
```

---

## Documentation

### Cross-component

| Document | What it covers |
|:--|:--|
| [docs/server.md](docs/server.md) | Server architecture, AI protocol, GUI protocol, world rules |
| [docs/ai_documentation.md](docs/ai_documentation.md) | AI strategy (Queen/Follower FSM), architecture deep-dive, testing |

### GUI (developer docs)

| Document | What it covers |
|:--|:--|
| [gui/docs/ARCHITECTURE.md](gui/docs/ARCHITECTURE.md) | System overview, frame loop, data ownership, ID allocation |
| [gui/docs/CLASS_DIAGRAM.md](gui/docs/CLASS_DIAGRAM.md) | Mermaid class diagrams for all subsystems |
| [gui/docs/ENTITY_BEHAVIOR.md](gui/docs/ENTITY_BEHAVIOR.md) | How to create entities, write behaviors, use EntityBuilder |
| [gui/docs/HUD.md](gui/docs/HUD.md) | How to add HUD panels, providers, and UI element types |
| [gui/docs/EVENTS.md](gui/docs/EVENTS.md) | All event types, how to handle and add new ones |
| [gui/docs/NETWORK.md](gui/docs/NETWORK.md) | Protocol reference, adding new server commands |
| [gui/docs/CAMERA.md](gui/docs/CAMERA.md) | Camera modes, map layouts, PlayerTileSystem |
| [gui/docs/GRAPHICS.md](gui/docs/GRAPHICS.md) | Renderer abstraction, shaders, grass, skybox |
| [gui/docs/AUDIO.md](gui/docs/AUDIO.md) | Spatial audio system, sound kinds, configuration |
| [gui/docs/MENU.md](gui/docs/MENU.md) | Menu scene, connect/launch panels, InputTextData |
| [gui/docs/LOGGER.md](gui/docs/LOGGER.md) | Logging API and sink configuration |

### AI (component README)

| Document | What it covers |
|:--|:--|
| [ai/README.md](ai/README.md) | Build, usage, quick start, debug, test commands |

---

## Contributing

See [docs/pr_format.md](docs/pr_format.md) for the pull request format used on this project.
