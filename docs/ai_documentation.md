# Zappy - AI Documentation

> **A complete guide to the AI client**: architecture, strategy, build, test, and integration.

---

## Table of Contents

1. [Overview](#overview)
2. [What is an AI in this context?](#what-is-an-ai-in-this-context)
3. [Finite State Machines (FSM)](#finite-state-machines-fsm)
4. [Project Structure](#project-structure)
5. [Build & Setup](#build--setup)
6. [Launching the Full Stack](#launching-the-full-stack)
7. [Strategy: From Selfish to Queen](#strategy-from-selfish-to-queen)
   - [V1 - The Selfish AI](#v1---the-selfish-ai)
   - [V2 - Adding Empathy](#v2---adding-empathy)
   - [V3 - The Queen Strategy (Final)](#v3---the-queen-strategy-final)
8. [Architecture Deep Dive](#architecture-deep-dive)
   - [Queen FSM](#queen-fsm)
   - [Follower FSM](#follower-fsm)
   - [Communication Protocol](#communication-protocol)
9. [Testing](#testing)
   - [Unit Tests with pytest](#unit-tests-with-pytest)
   - [Integration Testing with the Server and GUI](#integration-testing-with-the-server-and-gui)
10. [Configuration & Debug](#configuration--debug)

---

## Overview

The Zappy AI client (`zappy_ai`) is an **autonomous agent** that connects to the Zappy game server and pilots a player (called a *Trantorian*) through the game world. The goal is for at least **6 players from the same team to reach elevation level 8** - the maximum level.

Once launched, the AI requires **no human intervention**. It observes its environment, communicates with teammates via encrypted broadcasts, and makes decisions autonomously based on its internal state.

---

## What is an AI in this context?

In Zappy, an AI client is any **autonomous program** that connects to the server and pilots a player without human input. The implementation is intentionally unconstrained - valid approaches include rule-based systems, finite state machines, reinforcement learning, neural networks, or any combination thereof.  
This implementation uses a **rule-based FSM**: the agent perceives its environment through game commands (`Look`, `Inventory`, `Broadcast`), evaluates its current state, and emits commands according to a predefined set of rules. No training data or model weights are involved.

The agent follows a **sense → decide → act** loop:

``` diagram
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│  Perceive   │────▶│    Decide    │────▶│    Act      │
│ (Look, Inv) │     │  (FSM tick)  │     │ (commands)  │
└─────────────┘     └──────────────┘     └─────────────┘
       ▲                                        │
       └────────────────────────────────────────┘
```

Each agent maintains its own internal model of the world (position, inventory, vision, map memory) and communicates with teammates through the server's `Broadcast` mechanism.

---

## Finite State Machines (FSM)

The decision-making engine of each AI is a **Finite State Machine (FSM)**.

An FSM is a mathematical model of computation consisting of:

- A finite set of **states** (e.g., `Survive`, `Collect`, `GatherFollowers`)
- **Transitions** between states triggered by conditions
- An **active state** at any given time

``` diagram
           food < LOW            food >= HIGH
  ┌──────────────────────────────────────────┐
  ▼                                          │
Survive ──────────────────────────────▶ Collect
  ▲                                          │
  │         elevation_target set             │
  │   ┌─────────────────────────────────┐    │
  └───│            Gather               │◀───┘
      └─────────────────────────────────┘
               │ elevation_joined
               ▼
             Joined  ──── timeout ────▶ Collect
```

Each state implements three methods:

- `enter(ctx)` - called once when the state becomes active
- `update(ctx)` - called every tick; returns `None` to stay, or a new `State` to transition
- `exit(ctx)` - called once when leaving the state

This clean separation makes behavior **predictable**, **testable**, and **extensible**.

---

## Project Structure

``` architecture
ai/
├── main.py                  # Entry point
├── parser/
│   └── args.py              # CLI argument parsing
├── network/
│   ├── socket.py            # Low-level TCP socket wrapper
│   ├── zappyClient.py       # Game protocol client (handshake, commands, events)
│   ├── broadcast.py         # Message encoding/decoding
│   └── encryptor.py         # Fernet symmetric encryption
├── player/
│   ├── player.py            # Player state (level, inventory, position, vision)
│   ├── context.py           # BotContext: shared state passed to all FSM states
│   ├── fsm.py               # FSM base classes (State, FSM)
│   ├── enum.py              # Direction, Movement enums
│   ├── commands.py          # Command builder (Forward, Look, Take, ...)
│   ├── vision.py            # Vision and Tile data structures
│   ├── map.py               # Map memory
│   ├── navigation.py        # Pathfinding on toroidal map
│   ├── process.py           # Process fork/spawn wrapper
│   ├── zappyAI.py           # Top-level AI orchestrator
│   ├── states/
│   │   ├── discovery.py     # DiscoveryState: queen election at startup
│   │   └── follower.py      # Follower FSM states
│   └── queen/
│       └── state.py         # Queen FSM states
├── logger/
│   └── logger.py            # CSV event logger
└── logs/                    # Auto-generated log files (per run)
```

---

## Build & Setup

### Prerequisites

- Python 3.10+
- [`uv`](https://github.com/astral-sh/uv) (fast Python package manager)
- GNU `make`

### Install `uv`

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

### Option A - Build via Makefile (recommended)

From the **project root**:

```bash
make zappy_ai
```

This compiles/prepares the AI binary using the project Makefile. The resulting binary is `zappy_ai`.

### Option B - Run directly with `uv`

`uv` handles virtual environment creation and dependency installation automatically.

```bash
# Sync dependencies (first time or after changes)
uv sync

# Run directly
uv run ai/main.py -p 4242 -n MY_TEAM
```

> You do **not** need to manually activate a virtual environment. `uv run` handles it.

---

## Launching the Full Stack

### 1. Start the server

```bash
./zappy_server -p 4242 -x 20 -y 20 -n MY_TEAM -c 6 -f 100
```

| Flag | Description |
| ------ | ------------- |
| `-p` | Port number |
| `-x` / `-y` | Map dimensions |
| `-n` | Team name(s) |
| `-c` | Initial slots per team |
| `-f` | Frequency (time units per second) |

### 2. Start the GUI (optional but recommended)

```bash
./zappy_gui -p 4242
```

### 3. Launch the AI

Using the compiled binary:

```bash
./zappy_ai -p 4242 -n MY_TEAM
```

Using `uv` directly:

```bash
uv run ai/main.py -p 4242 -n MY_TEAM
```

#### Full CLI reference

``` helper
USAGE: ./zappy_ai -p port -n name1 name2 ... -h machine [options]

  -p port       Port number of the zappy server
  -n name       Team name(s) to join (one genesis AI per name)
  -h machine    Hostname of the server (default: localhost)
  --no-encrypt  Disable broadcast encryption
  -d / --debug  Enable verbose debug output
```

#### Debug mode

```bash
./zappy_ai -p 4242 -n MY_TEAM -d
```

Or via environment variable:

```bash
ZAPPY_AI_DEBUG=1 ./zappy_ai -p 4242 -n MY_TEAM
```

Debug output includes: tick counter, role (Q=Queen / F=Follower), level, food, position, FSM state, and action queue length.

---

## Strategy: From Selfish to Queen

The AI strategy went through three major iterations before reaching its final form.

---

### V1 - The Selfish AI

The first version gave every AI the same simple goal: **survive, collect resources, and attempt elevation alone**. The FSM had three states:

``` diagram
Survive ──▶ Collect ──▶ Incantation
  ▲                          │
  └──────────────────────────┘
```

**Survival** and **Collection** worked well - they are independent tasks that require no coordination. However, **incantation** is fundamentally a group activity. From level 2 onward, a player needs multiple teammates on the same tile with the correct stones. The selfish AI had no notion of helping others:

- Every agent independently collected **all** the stones needed for the ritual, causing massive **duplication of effort** - multiple AIs hoarding the same resources.
- When incantation time came, the AIs had no reliable way to meet on the same tile.
- Each AI wanted to be the one initiating the incantation (the "main character"), so coordination was chaotic.

**Outcome**: The team stagnated at low levels. Higher elevations were rarely achieved due to the near-impossibility of unassisted synchronization.

---

### V2 - Adding Empathy

To fix coordination, we added an **empathy mechanism**: when an AI received an `INCANT` broadcast from a teammate signaling an ongoing ritual, it would abandon its current task and move toward the broadcaster to join the incantation.

This felt like the right solution - and it worked for low-level rituals. But a subtle problem emerged:

**The AI had too much empathy.**

Whenever a *new* `INCANT` broadcast was received, the follower would drop everything - including a ritual it was already moving toward - and redirect toward the new one. In a team of 6+ agents all broadcasting incantation requests, agents were **perpetually redirected**, never settling on a single rally point.

We couldn't find the right balance: too little empathy and agents ignore teammates; too much and they become indecisive. The core issue was the **lack of a single authority** to coordinate the group.

**Outcome**: Slightly better than V1 for early levels, but still unreliable for level 6+ where 6 players must gather simultaneously.

---

### V3 - The Queen Strategy (Final)

The final strategy introduces a **hierarchical model** with one **Queen** and multiple **Followers**.

#### Queen Election

When an AI first connects, it enters `DiscoveryState` and broadcasts a `PING` message asking "is there already a queen?". It then waits up to 25 ticks for a `PONG` response:

- If a `PONG` arrives → the AI becomes a **Follower**.
- If no `PONG` arrives within the timeout → the AI declares itself **Queen** and broadcasts `PONG` to notify others.

If the Queen dies, the oldest surviving AI (the next one to time out without a PONG) becomes the new Queen.

#### Responsibilities

| Role | Responsibilities |
| ------ | ----------------- |
| **Queen** | Fork new AIs, poll follower status, collect global inventory, decide when and where to incant |
| **Follower** | Survive, collect resources, report inventory to queen, rally to queen on `INCANT` signal |

#### Why This Works

**Single authority**: Only the Queen broadcasts `INCANT` signals. Followers listen to exactly one leader, eliminating the "who do I follow?" confusion from V2.

**Global inventory awareness**: The Queen receives periodic `ALIVE` reports from all followers containing their full inventory. This gives the Queen a **global view of all resources**, allowing it to decide "we collectively have enough stones for the next ritual" rather than requiring every individual to independently hoard all required stones. This eliminates duplication of effort.

**Optimized stone collection**: In V1 and V2, each AI needed to collect *all* the stones for the incantation individually. With the Queen strategy, the collective inventory is considered - one follower might carry the linemate while another carries the sibur. The Queen checks whether the *team* has enough, then directs followers to drop their stones on the ritual tile.

**Controlled reproduction**: Only the Queen issues `Fork` commands, preventing uncontrolled population growth and ensuring that new slots are created strategically.

**Predictable rally behavior**: When the Queen is ready to incant, it continuously rebroadcasts `INCANT` with its position. Followers navigate toward the Queen and confirm arrival with a `JOIN` message. The Queen waits until enough followers have joined before triggering the incantation. Followers that have already joined simply wait - they do not get confused by new broadcasts.

---

## Architecture Deep Dive

### Queen FSM

``` diagram
QueenState
    └── _QS_Idle ◀──────────────── _QS_Incanting ──────────────────┐
          │                              ▲                         │
          │  food < LOW                  | enough followers joined │
          ▼                              |                         │
      _QS_Survive               _QS_GatherFollowers                │
          │                              ▲                         │
          │ food >= HIGH                 │ enough ressources       │
          │                              |                         │
          └─────────────────────▶ can_incant() == True ────────────┘

```

- **`_QS_Idle`**: Manages polling followers, spawning new bots (fork or connect), collecting food and stones, and evaluating incantation readiness.
- **`_QS_Survive`**: Prioritizes food collection when the Queen is low on resources.
- **`_QS_GatherFollowers`**: Stays in place, continuously broadcasts `INCANT`, waits for enough followers to join, and feeds itself from the current tile.
- **`_QS_Incanting`**: Drops required stones, sends `Incantation` command, waits for result, broadcasts `DONE`.

#### Incantation readiness check (`_can_incant`)

The Queen only initiates a ritual when **all** of the following are true:

1. Queen has `food >= FOOD_HIGH` (35 units)
2. At least `FOLLOWERS_NEEDED` (5) followers are alive and at the correct level
3. Every follower has `food >= FOOD_INCANT` (30 units)
4. The **global inventory** (Queen + all followers) contains enough stones for the next elevation

---

### Follower FSM

``` diagram
FollowerMainState
    └── _FS_Collect ◀───── _FS_Joined (timeout / level change) ────┐
          │                              ▲                         │
          │  food < LOW                  | elevation_joined        │
          ▼                              |                         │
      _FS_Survive                    _FS_Gather                    │
          │                              ▲                         │
          │ food >= HIGH                 │                         │
          │                              |                         │
          └─────────────────────▶ elevation_target set ────────────┘

```

- **`_FS_Collect`**: Default state. Collects food if needed, then collects missing stones for the next elevation level. Wanders if nothing is visible.
- **`_FS_Survive`**: Emergency food collection when critically low.
- **`_FS_Gather`**: Navigates toward the Queen's position using direction signals from `INCANT` broadcasts. Abandons if direction goes stale for too long.
- **`_FS_Joined`**: Waits on the Queen's tile. Drops required stones. Broadcasts `JOIN` to confirm arrival. Leaves if food drops or the ritual doesn't start in time.

---

### Communication Protocol

All messages are sent via the game's `Broadcast` command and encrypted with **Fernet symmetric encryption** (one shared key per team, generated at startup by the genesis AI and passed to all forked children via process arguments).

| Message | Direction | Payload | Purpose |
| --------- | ----------- | --------- | --------- |
| `PING` | Genesis → all | - | "Is there a queen?" |
| `PONG` | Queen → all | - | "I am the queen" |
| `POLL` | Queen → all | - | "Send me your status" |
| `ALIVE` | Follower → Queen | uuid, level, inventory | Full status report |
| `INCANT` | Queen → all | level, missing_count, x, y | "Rally to my tile for incantation" |
| `JOIN` | Follower → Queen | uuid, level | "I am on your tile" |
| `DONE` | Queen → all | new_level, x, y | "Incantation complete" |
| `RESOURCE` | Follower → Queen | resource, x, y | "I spotted a resource here" |

Message format: `TEAM_NAME:MSG_TYPE:payload` - encrypted before sending, decrypted on receipt.

---

### Navigation

The map is **toroidal** (wraps at edges). Pathfinding uses shortest-path delta calculation on each axis independently:

```python
def _shortest_delta(a, b, size):
    direct = (b - a) % size
    return direct if direct <= size // 2 else direct - size
```

The navigator moves along the X axis first, then Y, emitting `Forward`, `Left`, and `Right` commands to reach the target.

---

## Testing

### Unit Tests with pytest

Tests are located in the `ai/tests/` directory (or `tests/` depending on project layout).

#### Install test dependencies

```bash
uv sync --group dev
```

#### Run all tests

```bash
uv run pytest
```

#### Run a specific test file

```bash
uv run pytest tests/test_navigation.py
```

#### Run with verbose output

```bash
uv run pytest -v
```

#### Run with coverage report

```bash
uv run pytest --cov=ai --cov-report=term-missing
```

#### Key test areas

| Module | What to test |
| -------- | ------------- |
| `player/player.py` | `update_inventory`, `parse_look`, `can_elevate`, `_update_player_placement` |
| `player/navigation.py` | `navigate` with wrapping, all four directions |
| `player/vision.py` | `find_nearest`, `get_xy_tile`, `find_all` |
| `network/broadcast.py` | `encode`/`decode` round-trips, malformed input |
| `network/encryptor.py` | Encrypt/decrypt round-trip, wrong key returns `None` |
| `player/fsm.py` | State transitions, `enter`/`exit` call ordering |

#### Example test

```python
# tests/test_navigation.py
from player.navigation import navigate
from player.enum import Direction

def test_navigate_east():
    cmds = navigate((0, 0), Direction.NORTH, (3, 0), (10, 10))
    assert "Forward" in cmds
    assert cmds.count("Forward") == 3

def test_navigate_wraps():
    # From x=9 to x=1 on a width-10 map: shortest path goes east (wraps)
    cmds = navigate((9, 0), Direction.EAST, (1, 0), (10, 10))
    assert cmds.count("Forward") == 2  # 9→0→1 = 2 steps east
```

---

### Integration Testing with the Server and GUI

Integration testing verifies the full pipeline: server → AI → GUI.

#### Minimal integration setup

```bash
# Terminal 1: Start server (small map, high frequency for fast testing)
./zappy_server -p 4242 -x 10 -y 10 -n TEST -c 6 -f 200

# Terminal 2: Start GUI to observe behavior
./zappy_gui -p 4242 -h localhost

# Terminal 3: Launch the AI with debug enabled
./zappy_ai -p 4242 -n TEST -h localhost -d
```

#### What to observe

| Behavior | Expected |
| ---------- | ---------- |
| Startup | One AI becomes Queen (Q in debug), others become Followers (F) |
| Discovery | PING sent, PONG received within ~25 ticks |
| Forking | Queen forks new bots until `FOLLOWERS_NEEDED` (5) are alive |
| Food management | No bot dies of starvation (food should stay above critical) |
| Incantation rally | On INCANT broadcast, followers navigate toward Queen |
| Level progression | All bots gradually rise from level 1 to 8 |
| Win condition | Server reports end when 6 players reach level 8 |

#### Testing with `--no-encrypt`

To simplify debugging (readable broadcast messages in server logs):

```bash
./zappy_ai -p 4242 -n TEST --no-encrypt -d
```

#### Testing multiple teams

```bash
# Server with two competing teams
./zappy_server -p 4242 -x 20 -y 20 -n TEAM_A TEAM_B -c 6 -f 100

# Launch each team's AI
./zappy_ai -p 4242 -n TEAM_A &
./zappy_ai -p 4242 -n TEAM_B &
```

Since broadcasts are encrypted per-team (different keys), teams cannot intercept each other's communications.

---

## Configuration & Debug

### Environment Variables

| Variable | Default | Description |
| ---------- | --------- | ------------- |
| `ZAPPY_AI_DEBUG` | `0` | Set to any non-zero value to enable debug output |

### Tunable Constants

These are defined in `player/queen/state.py` and `player/context.py`:

| Constant | Default | Description |
| ---------- | --------- | ------------- |
| `FOOD_CRITICAL` | 5 | Emergency threshold - stop everything and eat |
| `FOOD_LOW` | 10 | Prefer eating over collecting stones |
| `FOOD_INCANT` | 30 | Minimum food for a follower to join an incantation |
| `FOOD_HIGH` | 35 | Comfortable - can focus on stones or rituals |
| `FOLLOWERS_NEEDED` | 5 | Number of followers required for a ritual |
| `POLL_INTERVAL` | 20 | Ticks between Queen status polls |
| `FORK_COOLDOWN` | 20 | Minimum ticks between fork attempts |
| `FOLLOWER_JOIN_TIMEOUT` | 600 | Ticks before Queen abandons a rally attempt |
| `INVENTORY_CHECK_INTERVAL` | 15 | Ticks between local inventory refreshes |

### Log Files

Each run generates a CSV log file in `ai/logs/run_<timestamp>/`:

``` log
logs_MY_TEAM_<pid>.csv
```

Columns: `tick, player_id, level, status, linemate, deraumere, sibur, mendiane, phiras, thystame, chosen_action, action_success`

These logs are useful for post-mortem analysis of bot behavior, starvation events, and incantation failures.
