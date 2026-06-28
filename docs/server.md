# Zappy Server

The `zappy_server` is the authoritative game engine for Zappy. It owns the world
state, runs the simulation loop, and speaks two TCP line protocols: one for AI
players (`ai`) and one for graphical clients (`gui`). Every rule — movement,
resource consumption, incantations, the win condition — is enforced here; clients
only send commands and render what the server reports.

- Source: `server/`
- Binary: `zappy_server` (also copied to `server/build/bin/`)
- Language: C (C99-ish, `-Wall -Wextra`), POSIX sockets + `poll(2)`

---

## Build & run

```sh
cd server
make              # builds build/bin/zappy_server and copies it to ./zappy_server
make re           # full rebuild
make clean        # remove build artifacts
make fclean       # clean + remove binaries
make tests_run    # build and run the unit tests (./unit_tests)
make compile_commands.json   # generate compile_commands.json via bear
```

### Usage

```
./zappy_server -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq
./zappy_server --help
```

| Flag | Meaning | Mandatory | Default |
|------|---------|-----------|---------|
| `-p` | Listening port (1–65535) | yes | — |
| `-x` | Map width (tiles) | yes | — |
| `-y` | Map height (tiles) | yes | — |
| `-n` | Space-separated team names | yes | — |
| `-c` | Slots per team (max clients per team) | yes | — |
| `-f` | Tick frequency (time unit divisor) | no | `100` |

`-f` sets the simulation speed: a command costing `N` time units takes
`N / freq` seconds. Higher `freq` ⇒ faster game. Exit code is `0` on success,
`84` on failure (`EXIT_FAILED`).

Argument parsing lives in `src/components/parsing/` (`getopt`-based). Missing
mandatory flags, bad ports, or non-numeric integers abort startup with an error
on `stderr`.

---

## Architecture overview

```
main.c
  └─ handle_pre_serv_proc()                 (src/components/init_prog.c)
       ├─ create permanent memory arena (1 MB)
       ├─ parse_cmd_line()                  (parsing)
       ├─ init_world_map()                  (world)
       ├─ init_server()                     (server/init_server.c)
       └─ run_server()                      (server/server.c)
            └─ main loop: net_poll() + game_tick()
```

The codebase is organised under `src/components/`:

| Directory | Responsibility |
|-----------|----------------|
| `init_prog.c` | Top-level startup orchestration |
| `parsing/` | Command-line argument parsing |
| `world/` | Map allocation, resource scatter/refill |
| `server/` | Socket setup, poll loop, client lifecycle |
| `server/connection/` | Handshake, client-state transitions, player stats, respawn |
| `server/client_commands/player_commands/` | AI command handlers |
| `server/client_commands/gui_commands/` | GUI request + push handlers |
| `server/io/` | `event_sink` — outbound message fan-out |
| `game/` | `game_tick()` — one simulation step |
| `lib/` | Arena allocator, vectors, math, enum→string helpers |
| `errors/` | Centralised error-code → string tables |

Public types and prototypes are in `include/` (`server.h`, `commands.h`,
`world.h`, `config.h`, `protocol.h`, `arena.h`, `vector.h`, `event_sink.h`).

### Memory model

Two allocation strategies coexist:

- **Arena allocator** (`arena.h`, `lib/mem_arena.c`): a bump allocator. One
  permanent 1 MB arena (`prog_cfg.perm_mem_arena`) holds long-lived structures
  (server, teams, world tiles). Each client also owns a small arena for its
  team name and per-command scratch buffers. Freed all at once.
- **Dynamic vectors** (`vector.h`): macro-generated growable arrays
  (`vec_client_t`, `vec_egg_t`, `vec_pollfd_t`) backed by `malloc`/`realloc`,
  used for the runtime-sized collections of clients, eggs and pollfds.

`vector_remove` is swap-with-last (O(1), unordered) — iteration code decrements
its index after a removal so it doesn't skip the swapped-in element.

---

## The main loop

`run_server()` (`server/server.c`) installs signal handlers, then loops until a
shutdown is requested:

```c
while (!shutdown_requested()) {
    net_poll(server);          // I/O: accept + read clients (100 ms poll timeout)
    if (game_tick(server)) break;   // simulation: stats, commands, win, respawn
}
server_shutdown(server);
```

### Networking (`net_poll`)

`poll(2)` watches the listener fd plus every client fd with a **100 ms** timeout.
On `POLLIN`:

- listener fd → `handle_server_reception()`: `accept`, send `WELCOME\n`, create a
  client in the `homeless` vector (not yet typed), register its pollfd.
- client fd → `handle_client_reception()`: drains the socket, parsing one line at
  a time and dispatching each.

`EINTR` from `poll` is ignored (signal during wait). Read/parse helpers live in
`server/helpers/parse_packets.c`.

### Simulation (`game_tick`, `game/game.c`)

```c
int game_tick(server_t *server) {
    handle_player_stats(server);       // hunger countdown, deaths, action cooldowns
    handle_commands_dispatch(server);  // execute one queued command per player if ready
    if (handle_win_con(server)) return 1;
    handle_resource_respawn(server);   // periodic map refill
    server->uptime += 1;
    return 0;
}
```

Timing is wall-clock based via `now_ms()` (`CLOCK_MONOTONIC`), so the game speed
is decoupled from how often the loop spins.

---

## Connection lifecycle

A client moves through `ClientState`: `PENDING` → `CONN` (or `REJECTED`).

1. **Accept** — server sends `WELCOME\n`. The client sits in `homeless`,
   untyped, in `PENDING`.
2. **Handshake** (`connection/handle_client_state.c`,
   `handle_client_handshake`) — the first line the client sends decides its kind:
   - `GRAPHIC` → becomes a **GUI** client. The server immediately streams the
     full world state (`msz`, `sgt`, `mct`, `tna`, then per-player `pnw`/`plv`/`pin`).
   - any other string is treated as a **team name** → becomes a **PLAYER**.
3. **Player admission** — `find_team` then `check_team_slot`:
   - unknown team or no free slot → `reject_client` (`ko`).
   - accepted → the player spawns (random tile/direction) **or** hatches from an
     existing team egg if one is queued. The server replies with
     `<remaining-slots>\n<width> <height>\n` and notifies GUIs via `pnw`.
4. **Promotion** — once a `homeless` client reaches `CONN`, it is moved into the
   typed `players` or `guis` vector (`move_homeless_client`).

New players start at level 1 with `inv[FOOD] = 10` and a food deadline.

Disconnects (`disconnect_client`) close the fd, remove the pollfd, free the
client arena, and emit the appropriate GUI death/expel notifications.

---

## Protocol & message parsing

Lines are terminated by `\n` (LF) or `\r\n` (CRLF, for raw telnet). The parser
(`recv_parser` / `flush_current_buffer`) keeps a 4096-byte per-client receive
buffer (`RECEPTION_SIZE`), extracts complete lines, and leaves partial input
buffered across reads. Multiple commands received in one packet are processed in
sequence within `handle_client_reception`.

Standard replies: `ok\n` (success), `ko\n` (failure / bad params / unknown
command), `dead\n` (player starved).

### Command flow

- **Players**: incoming lines are *queued* per client (`push_commands`, max
  `MAX_COMMANDS = 10` pending). `handle_commands_dispatch` pops and executes at
  most one queued command per player per tick, and only when the player's
  cooldown has elapsed. Excess commands beyond 10 are dropped — this is the
  protocol's command buffering limit.
- **GUIs**: handled immediately on receipt (no queue), since GUI commands are
  read-only queries.

Outbound messages go through the **event sink** (`io/event_sink.c`,
`event_sink.h`): `sink_to_player`, `sink_to_fd`, `sink_to_guis`,
`sink_player_pos`. This centralises sends and GUI fan-out so handlers don't
duplicate socket logic.

---

## AI (player) commands

Dispatched through a table in
`player_commands/handle_player_commands.c`. Each command costs a number of
**time units**; the real cooldown in ms is `cd * 1000 / freq`
(`set_client_cd`). While on cooldown a player's further commands stay queued.

| Command | Time units | Effect |
|---------|-----------:|--------|
| `Forward` | 7 | Move one tile in the facing direction (toroidal wrap) |
| `Right` | 7 | Turn 90° clockwise |
| `Left` | 7 | Turn 90° counter-clockwise |
| `Look` | 7 | Return the cone of tiles the player can see |
| `Inventory` | 1 | Return resource counts + remaining food |
| `Broadcast <text>` | 7 | Send `text` to all players with a direction code |
| `Connect_nbr` | 0 | Return free team slots |
| `Fork` | 42 | Lay an egg (adds a future team slot) |
| `Eject` | 7 | Push players off the current tile |
| `Take <obj>` | 7 | Pick a resource up off the tile |
| `Set <obj>` | 7 | Drop a resource onto the tile |
| `Incantation` | 300 | Begin an elevation ritual |

Cooldown constants live in `commands.h` (`CD_FORWARD`, … `CD_INCANTATION`).

### Vision (`Look`)

`look.c` builds the classic Zappy vision cone: row `0` is the current tile, each
subsequent row `r` (out to `r = level`) widens by `2r+1` tiles, oriented to the
player's facing direction with toroidal wrapping (`wrap`). Per tile it lists
`player` tokens and each resource by name, tiles separated by `, ` inside
`[ ... ]`. Vision depth equals the player's level.

### Hunger & death (`connection/handle_player.c`)

Each player burns 1 food every `126 / freq` seconds (`food_dealine`). When food
hits 0 the player is sent `dead\n`, the GUI gets `pdi`, and the client is
disconnected. Cooldowns decrement by real elapsed ms each `handle_player_stats`
pass.

### Incantation / elevation (`incantation.c`)

`handle_incantation` checks the level requirements (player count *and* tile
resources) from the `ELEV_REQ[7][7]` table. If met, all eligible same-level
players on the tile enter the `CHANNELING` state with a 300-time-unit cooldown
and receive `Elevation underway`. After the cooldown, `finish_incantation`
re-verifies the prerequisites, consumes the tile resources, raises every
participant's level, and notifies the GUI (`pie`, `plv`). Reaching level 8 bumps
the team's `max_lvls` counter (used by the win check).

Elevation requirements (`[level→level+1] = players, lin, der, sib, men, phi, thy`):

| Level → | Players | Linemate | Deraumere | Sibur | Mendiane | Phiras | Thystame |
|--------:|:-------:|:--------:|:---------:|:-----:|:--------:|:------:|:--------:|
| 1→2 | 1 | 1 | 0 | 0 | 0 | 0 | 0 |
| 2→3 | 2 | 1 | 1 | 1 | 0 | 0 | 0 |
| 3→4 | 2 | 2 | 0 | 1 | 0 | 2 | 0 |
| 4→5 | 4 | 1 | 1 | 2 | 0 | 1 | 0 |
| 5→6 | 4 | 1 | 2 | 1 | 3 | 0 | 0 |
| 6→7 | 6 | 1 | 2 | 3 | 0 | 1 | 0 |
| 7→8 | 6 | 2 | 2 | 2 | 2 | 2 | 1 |

### Fork & eggs

`Fork` queues an egg (`vec_egg_t`) at the player's position and grants the team
an extra slot. New connections for that team hatch from the oldest queued egg
before falling back to a random spawn. GUI sees `pfk` then `enw`.

---

## GUI commands

The GUI speaks its own protocol (handshake string `GRAPHIC`). Requests are
dispatched in `gui_commands/handle_gui_commands.c`:

| Request | Meaning |
|---------|---------|
| `msz` | Map size |
| `bct X Y` | Tile content |
| `mct` | Whole map content |
| `tna` | Team names |
| `ppo #id` | Player position |
| `plv #id` | Player level |
| `pin #id` | Player inventory |
| `sgt` | Get time-unit (frequency) |
| `sst T` | Set time-unit (frequency) |
| `stu` | (custom) server/time status |

Beyond replies to requests, the server **pushes** unsolicited events to all GUIs
as the game changes (`passive_commands/`): `pnw` (new player), `pdi` (death),
`pdr`/`pgt` (drop/take), `pbc` (broadcast), `pic`/`pie` (incantation start/end),
`pfk` (fork), `enw`/`ebo`/`edi` (egg new/hatch/death), `seg` (game end), `smg`
(server message), `pex` (expulsion). These flow through `group_gui_send` /
`sink_to_guis`. Wire details for both protocols are in the PDFs under `docs/`.

---

## World & resources

The map is a flat `w * h` array of `map_tile_t` (`world.h`), each tile holding a
per-resource count plus player/egg counts. The map is **toroidal** — movement
and vision wrap on both axes.

Resources are scattered by target density (`world/world.c`):

| Resource | Density |
|----------|--------:|
| Food | 0.5 |
| Linemate | 0.3 |
| Deraumere | 0.15 |
| Sibur | 0.1 |
| Mendiane | 0.1 |
| Phiras | 0.08 |
| Thystame | 0.05 |

`refill_world_resources` computes the target count per resource
(`w * h * density`, min 1) and scatters only the **deficit**, so picking objects
up lets the world top back up over time. Refill runs every `20 / freq` seconds
(`RESPAWN_INTERVAL`, `handle_resource_respawn`) and notifies GUIs via `mct`.

---

## Win condition

`handle_win_con` (`win_condition.c`) declares a winner when a team has at least
**6** players who reached level **8** (`max_lvls >= MIN_PLAYERS_MAX`). The server
sends `seg <team>` to GUIs, mass-disconnects everyone, closes the listener, and
`game_tick` returns 1 to end the loop.

---

## Shutdown

`SIGINT`/`SIGTERM` (`server/signal_handler.c`) set a flag polled by
`shutdown_requested()`. The loop then exits cleanly: `server_shutdown`
disconnects remaining clients, closes the listener, and `server_cleanup` frees
every vector. The permanent arena is freed last in `handle_pre_serv_proc`.

---

## Tests

`server/tests/` holds unit tests (memory arenas, command-line parsing). Run with
`make tests_run`. `server/tests_clients/` contains small standalone client
programs (`gui_client.c`, `player_client.c`, …) for manual end-to-end checks
against a running server.
