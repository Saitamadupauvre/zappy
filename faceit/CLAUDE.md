# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

FACEIT is a competitive matchmaking platform for Zappy. It orchestrates matches between uploaded AI binaries, tracks Elo ratings, and exposes a REST API + single-page portal. It does **not** modify `zappy_server` or `zappy_gui` — it drives them as subprocesses.

## Running (NixOS)

```bash
nix-shell          # enter env — provides Python 3.13 + all deps, no pip needed
make run           # uvicorn at http://localhost:8000 with --reload
```

`shell.nix` pins the full dependency set. Never use system `pip` or `python3 -m pip` on NixOS — it won't work.

## Architecture

```
Request → FastAPI router → service/matchmaker
                         → DB (AsyncSession / SQLite)

Queue full (4 AIs) → MatchmakerService._maybe_trigger()
                   → asyncio.create_task(GameInstance.run())
                        ├── find_free_port()
                        ├── spawn zappy_server subprocess
                        ├── spawn 4 AI subprocesses (team ai_0…ai_3, -c 1)
                        ├── PassiveSpectator.watch() — connects as GRAPHIC, reads until "seg <team>\n"
                        ├── terminate all subprocesses
                        └── EloEngine.update_match() → commit to DB
```

`MatchmakerService` is a singleton on `app.state.matchmaker` (set in lifespan). It holds an in-memory queue protected by `asyncio.Lock`. Match tasks are fire-and-forget — if the orchestrator restarts mid-match, those matches stay `status="running"` in the DB.

## Key Files

| File | Purpose |
|---|---|
| `orchestrator/main.py` | FastAPI app factory, lifespan (DB init + matchmaker), router mounts, static portal |
| `orchestrator/config.py` | `Settings` singleton — paths, secrets, map size, freq, queue threshold |
| `orchestrator/database.py` | Async SQLite engine, `get_session` dependency, `init_db()` |
| `orchestrator/models/` | SQLModel table definitions (User, AIBinary, Match, MatchParticipant) |
| `orchestrator/routers/auth.py` | `/api/register`, `/api/login` (JWT), `/api/me`, `get_current_user` dep |
| `orchestrator/routers/ai.py` | `/api/ai/upload` (multipart), `/api/ai/list`, `/api/ai/{id}` DELETE |
| `orchestrator/routers/queue.py` | `/api/queue/join`, `/api/queue/leave`, `/api/queue/status` |
| `orchestrator/routers/matches.py` | `/api/matches`, `/api/matches/live`, `/api/matches/{id}` |
| `orchestrator/routers/leaderboard.py` | `/api/leaderboard` — top 50 by MMR |
| `orchestrator/services/matchmaker.py` | `MatchmakerService` — in-memory queue, match triggering |
| `orchestrator/services/game_instance.py` | `GameInstance` — full match lifecycle (spawn → watch → finalize) |
| `orchestrator/services/mmr_engine.py` | `update_match()` — pairwise Elo, K=32, floor=100 |
| `orchestrator/utils/port_allocator.py` | `find_free_port()` — scans 4300–5000 |
| `orchestrator/utils/spectator.py` | `watch_for_winner()` — passive TCP GUI client watching for `seg` |
| `portal/` | Static single-page app served at `/` by FastAPI `StaticFiles` |

## Critical SQLAlchemy Note

**Always use `session.execute(select(...)).scalars()` — never `session.exec()`.**

`AsyncSession` from SQLAlchemy has no `exec` method. That's SQLModel's sync-only wrapper. Every query must follow this pattern:

```python
# correct
result = (await session.execute(select(User).where(User.username == name))).scalars().first()
rows   = (await session.execute(select(AIBinary).where(...))).scalars().all()

# wrong — AttributeError at runtime
result = (await session.exec(select(User).where(...))).first()
```

## Data Models

**User** — `id, username (unique), password_hash, created_at`

**AIBinary** — `id, user_id FK, name, file_path, mmr=1000.0, matches_played, uploaded_at, is_active`  
Uploaded binaries are stored at `faceit/uploads/<user_id>/<uuid>/binary` and chmod'd executable.

**Match** — `id, server_port, status, winner_team, started_at, finished_at`  
Status values: `pending` → `running` → `finished` | `error`

**MatchParticipant** — `id, match_id FK, ai_binary_id FK, team_name, mmr_before, mmr_after`  
`team_name` is `ai_0`…`ai_3` (one team per AI, `-c 1` on the server).

## Elo Formula

4-player free-for-all treated as pairwise: winner beats each of the 3 losers individually.

```python
K = 32.0
# For each (winner, loser) pair:
E_win = 1 / (1 + 10 ** ((loser_mmr - winner_mmr) / 400))
winner_delta += K * (1 - E_win)
loser_delta  += K * (0 - (1 - E_win))
# Floor: max(100.0, new_mmr)
```

## Match End Detection

`PassiveSpectator` connects to `zappy_server` as a GUI client (`GRAPHIC\n` handshake), then reads lines until it sees `seg <team_name>\n`. The server closes all connections immediately after broadcasting `seg`, so detection is clean — no polling or timeout needed for the happy path. `asyncio.wait_for(..., timeout=3600)` guards against hung matches.

Connection uses retry-with-backoff (up to 10 attempts, 0.5s × attempt) to handle server startup delay.

## Adding a New Endpoint

1. Add route to the relevant router in `orchestrator/routers/`
2. Use `get_current_user` dep from `routers/auth.py` for auth
3. Use `get_session` dep from `database.py` for DB access
4. Register the router in `main.py` if it's a new file

## Config

All tunable values are in `orchestrator/config.py` `Settings` class. Override `FACEIT_SECRET` env var in production. `server_binary` defaults to `../zappy_server` (relative to `faceit/`).
