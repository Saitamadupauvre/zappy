#!/usr/bin/env bash
# zappy_run.sh — one tool to launch the Zappy server, GUI, plain AI, and trained RL models.
#
# Subcommands:
#   setup           Create ai/.venv and install the RL deploy deps (jax/flax/numpy). One-time.
#   server          Build (if needed) and run the C server.
#   gui             Build (if needed) and run the C++ GUI, attaching to a server.
#   ai              Build (if needed) and run the hand-written stdlib AI client(s).
#   model           Start server + (GUI) + a team of trained RL agents from ai/runs. <- headline
#   demo            Alias for `model` with the GUI on (full stack in one shot).
#
# Run `zappy_run.sh <subcommand> --help` style flags are shared; see usage() below.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
AI_DIR="$ROOT/ai"
GUI_DIR="$ROOT/gui"
SERVER_DIR="$ROOT/server"
RUNS_DIR="$AI_DIR/runs"
VENV_PY="$AI_DIR/.venv/bin/python"

# ---- shared defaults -------------------------------------------------------
HOST="127.0.0.1"
PORT="4242"
TEAM="T1"
MODEL=""            # 10x10 | 30x30 shorthand (resolved via runs/models.json)
RUN_DIR=""          # explicit run directory under ai/runs (or absolute)
PARAMS=""
CONFIG=""
AGENTS=""           # default from config.json (n_agents) for model; 1 for ai
WIDTH=""            # default from config.json
HEIGHT=""           # default from config.json
FREQ=""             # default from config.json
ROLE_DIM=""         # default from config.json (role_dim) or 6
INV_EVERY=""        # default from config.json (inv_every) or 10
DURATION="600"      # 0 = run until death/disconnect
GREEDY="1"
START_SERVER="1"
START_GUI="1"
OUT_DIR=""
PYTHON_OVERRIDE=""
BUILD="1"           # rebuild the launched component(s) from source first (avoids stale binaries)

# process state (kept global so the EXIT trap can reach it after cmd_model returns)
SERVER_PID=""
GUI_PID=""
FIFO=""
AGENT_PIDS=()

usage() {
  cat <<'EOF'
Usage:
  scripts/zappy_run.sh <command> [options]

Commands:
  setup                 Create ai/.venv and install RL deps (numpy/jax/flax). Run once.
  server [options]      Run the C server (builds it if missing).
  gui [options]         Run the C++ GUI attached to a server (builds it if missing).
  ai [options]          Run the stdlib AI client(s) (builds zappy_ai if missing).
  model [options]       Server + GUI + a team of trained RL agents from ai/runs.
  demo [options]        Same as `model` with the GUI on.

Options (shared; sensible per-command defaults):
  --model 10x10|30x30   Pick a trained model by map size (via ai/runs/models.json).
  --run DIR             Run directory with params.msgpack/config.json (under ai/runs or absolute).
  --params FILE         Explicit checkpoint; default: RUN/params.msgpack
  --config FILE         Explicit config;     default: RUN/config.json
  --host HOST           Server host (default 127.0.0.1)
  --port PORT           Server port (default 4242)
  --team TEAM           Team name (default T1)
  --agents N            Number of agents/clients (default: model -> config n_agents; ai -> 1)
  --width W             Server map width  (default: model -> config width)
  --height H            Server map height (default: model -> config height)
  --freq F              Server frequency  (default: model -> config freq, else 100)
  --role-dim N          Role one-hot dim  (default: config role_dim, else 6)   [model]
  --inv-every N         Inventory cadence (default: config inv_every, else 10) [model]
  --duration SEC        Agent run time; 0 = until death/disconnect (default 600) [model]
  --stochastic          Sample policy actions instead of greedy argmax [model]
  --attach              Don't start a server; connect to an existing one [model/gui/ai]
  --no-gui              Don't launch the GUI [model]
  --gui                 Launch the GUI [model] (on by default)
  --no-build            Skip rebuilding the component(s) from source before launch
  --python PY           Python interpreter override (setup/model)
  -h, --help            Show this help

Examples:
  scripts/zappy_run.sh setup
  scripts/zappy_run.sh model --model 10x10
  scripts/zappy_run.sh model --model 30x30 --no-gui --duration 120
  scripts/zappy_run.sh model --run strategy-v2-s21-10x10-f100-ov10-ds0.85 --stochastic
  scripts/zappy_run.sh server --port 4242 --width 10 --height 10 --freq 100 --agents 6
  scripts/zappy_run.sh gui --port 4242
  scripts/zappy_run.sh ai --port 4242 --team T1 --agents 1
EOF
}

# ---- option parsing --------------------------------------------------------
parse_opts() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --model)      MODEL="$2"; shift 2 ;;
      --run)        RUN_DIR="$2"; shift 2 ;;
      --params)     PARAMS="$2"; shift 2 ;;
      --config)     CONFIG="$2"; shift 2 ;;
      --host)       HOST="$2"; shift 2 ;;
      --port)       PORT="$2"; shift 2 ;;
      --team)       TEAM="$2"; shift 2 ;;
      --agents)     AGENTS="$2"; shift 2 ;;
      --width)      WIDTH="$2"; shift 2 ;;
      --height)     HEIGHT="$2"; shift 2 ;;
      --freq)       FREQ="$2"; shift 2 ;;
      --role-dim)   ROLE_DIM="$2"; shift 2 ;;
      --inv-every)  INV_EVERY="$2"; shift 2 ;;
      --duration)   DURATION="$2"; shift 2 ;;
      --stochastic) GREEDY="0"; shift ;;
      --attach)     START_SERVER="0"; shift ;;
      --no-gui)     START_GUI="0"; shift ;;
      --gui)        START_GUI="1"; shift ;;
      --no-build)   BUILD="0"; shift ;;
      --build)      BUILD="1"; shift ;;
      --out-dir)    OUT_DIR="$2"; shift 2 ;;
      --python)     PYTHON_OVERRIDE="$2"; shift 2 ;;
      -h|--help)    usage; exit 0 ;;
      *) echo "unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
  done
}

# ---- small helpers ---------------------------------------------------------
log() { echo "[zappy] $*"; }
die() { echo "[zappy] error: $*" >&2; exit 1; }

# Read an integer from a JSON config using the system python3 (stdlib only).
read_cfg_int() {
  local path="$1" key="$2" fallback="$3"
  python3 - "$path" "$key" "$fallback" <<'PY'
import json, sys
path, key, fallback = sys.argv[1], sys.argv[2], sys.argv[3]
try:
    v = json.load(open(path)).get(key, None)
except Exception:
    v = None
print(fallback if v is None else int(v))
PY
}

# Rebuild a component from source (unless --no-build). Incremental: make/cmake
# only recompiles what changed, so this is cheap and prevents stale binaries.
build_component() {
  local label="$1" dir="$2"
  [[ "$BUILD" == "1" ]] || return 0
  local blog="${TMPDIR:-/tmp}/zappy_build_${label}.log"
  log "building $label (make -C $dir)..."
  if ! make -C "$dir" >"$blog" 2>&1; then
    cat "$blog" >&2
    die "build failed for $label (see $blog)"
  fi
}

# Resolve a binary from candidate paths; build via `make -C <dir>` if missing.
resolve_bin() {
  local name="$1" build_dir="$2"; shift 2
  local c
  for c in "$@"; do [[ -x "$c" ]] && { echo "$c"; return 0; }; done
  log "$name not found — building (make -C $build_dir)..." >&2
  make -C "$build_dir" >&2
  for c in "$@"; do [[ -x "$c" ]] && { echo "$c"; return 0; }; done
  return 1
}

resolve_server_bin() {
  resolve_bin zappy_server "$SERVER_DIR" \
    "$SERVER_DIR/zappy_server" "$ROOT/zappy_server" "$SERVER_DIR/build/bin/zappy_server"
}
resolve_gui_bin() {
  resolve_bin zappy_gui "$GUI_DIR" "$GUI_DIR/zappy_gui" "$ROOT/zappy_gui"
}
resolve_ai_bin() {
  # ai/Makefile writes ../zappy_ai (project root)
  resolve_bin zappy_ai "$AI_DIR" "$ROOT/zappy_ai" "$AI_DIR/zappy_ai"
}

# Block until host:port accepts a TCP connection (or time out).
wait_for_port() {
  python3 - "$HOST" "$PORT" <<'PY'
import socket, sys, time
host, port = sys.argv[1], int(sys.argv[2])
deadline = time.time() + 12.0
while time.time() < deadline:
    try:
        with socket.create_connection((host, port), timeout=0.3):
            raise SystemExit(0)
    except OSError:
        time.sleep(0.1)
raise SystemExit(1)
PY
}

# Resolve a RUN directory to an absolute path under ai/runs (or as given).
resolve_run() {
  if [[ -n "$MODEL" && -z "$RUN_DIR" ]]; then
    local mapped
    mapped="$(python3 - "$RUNS_DIR/models.json" "$MODEL" <<'PY'
import json, sys
try:
    print(json.load(open(sys.argv[1])).get(sys.argv[2], ""))
except Exception:
    print("")
PY
)"
    [[ -n "$mapped" ]] || die "unknown --model '$MODEL' (see $RUNS_DIR/models.json)"
    RUN_DIR="$mapped"
  fi
  [[ -n "$RUN_DIR" ]] || RUN_DIR="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1])).get("10x10",""))' "$RUNS_DIR/models.json" 2>/dev/null || true)"
  [[ -n "$RUN_DIR" ]] || die "no run selected; pass --model 10x10|30x30 or --run DIR"
  # Make absolute: try as-given, then under ai/runs.
  if [[ -d "$RUN_DIR" ]]; then RUN_DIR="$(cd "$RUN_DIR" && pwd)";
  elif [[ -d "$RUNS_DIR/$RUN_DIR" ]]; then RUN_DIR="$RUNS_DIR/$RUN_DIR";
  else die "run directory not found: $RUN_DIR"; fi
}

resolve_python() {
  if [[ -n "$PYTHON_OVERRIDE" ]]; then echo "$PYTHON_OVERRIDE"; return 0; fi
  if [[ -n "${ZAPPY_PYTHON:-}" ]]; then echo "$ZAPPY_PYTHON"; return 0; fi
  [[ -x "$VENV_PY" ]] && { echo "$VENV_PY"; return 0; }
  return 1
}

# The stdlib AI requires Python >=3.12; the default python3 may be older. Find a
# suitable interpreter so we can run the zipapp with it (bypassing its shebang).
py_ge_312() {
  local c
  for c in python3.13 python3.12 python3; do
    command -v "$c" >/dev/null 2>&1 || continue
    if "$c" -c 'import sys; raise SystemExit(0 if sys.version_info[:2] >= (3,12) else 1)' 2>/dev/null; then
      command -v "$c"; return 0
    fi
  done
  return 1
}

# ---- subcommands -----------------------------------------------------------
cmd_setup() {
  local py="${PYTHON_OVERRIDE:-python3}"
  command -v "$py" >/dev/null 2>&1 || [[ -x "$py" ]] || die "python not found: $py"
  log "creating venv at $AI_DIR/.venv"
  "$py" -m venv "$AI_DIR/.venv"
  log "installing RL deploy deps (numpy/jax/flax)..."
  "$VENV_PY" -m pip install --upgrade pip
  "$VENV_PY" -m pip install -r "$AI_DIR/rl/requirements.txt"
  log "verifying import + versions"
  PYTHONPATH="$AI_DIR" "$VENV_PY" -c \
    'import numpy, jax, flax, rl.deploy.zappy_ai_adapter as a; \
     print("ok: jax", jax.__version__, "flax", flax.__version__, "numpy", numpy.__version__)'
  log "setup complete — run: scripts/zappy_run.sh model --model 10x10"
}

cmd_server() {
  build_component server "$SERVER_DIR"
  local bin; bin="$(resolve_server_bin)" || die "could not build/find zappy_server"
  local w="${WIDTH:-10}" h="${HEIGHT:-10}" f="${FREQ:-100}" c="${AGENTS:-6}"
  log "server: ${w}x${h} port=$PORT freq=$f team=$TEAM clients=$c"
  exec "$bin" -p "$PORT" -x "$w" -y "$h" -n "$TEAM" -c "$c" -f "$f"
}

cmd_gui() {
  build_component gui "$GUI_DIR"
  local bin; bin="$(resolve_gui_bin)" || die "could not build/find zappy_gui"
  log "gui -> $HOST:$PORT"
  # run from gui/ so the GUI's relative "assets/..." paths (models/shaders/images) resolve
  cd "$GUI_DIR" && exec "$bin" -p "$PORT" -h "$HOST"
}

cmd_ai() {
  build_component ai "$AI_DIR"
  local bin; bin="$(resolve_ai_bin)" || die "could not build/find zappy_ai"
  local n="${AGENTS:-1}"
  # zappy_ai needs Python >=3.12; run it with a suitable interpreter if the
  # default python3 (its shebang) is older.
  local launch=("$bin") aipy
  if aipy="$(py_ge_312)"; then launch=("$aipy" "$bin"); else
    log "warning: no python>=3.12 found; zappy_ai may fail under the default python3"
  fi
  local pids=()
  cleanup_ai() { for p in "${pids[@]:-}"; do kill "$p" 2>/dev/null || true; done; }
  trap cleanup_ai EXIT INT TERM
  log "launching $n stdlib AI client(s) for team '$TEAM' -> $HOST:$PORT"
  for ((i = 0; i < n; i++)); do
    "${launch[@]}" -p "$PORT" -n "$TEAM" -h "$HOST" &
    local pid=$!
    pids+=("$pid")
    log "ai #$i pid=$pid"
  done
  wait
}

cmd_model() {
  resolve_run
  PARAMS="${PARAMS:-$RUN_DIR/params.msgpack}"
  CONFIG="${CONFIG:-$RUN_DIR/config.json}"
  [[ -f "$PARAMS" ]] || die "checkpoint not found: $PARAMS"
  [[ -f "$CONFIG" ]] || die "config not found: $CONFIG"

  # Fill unset params from the run's config so each model matches its map.
  [[ -n "$WIDTH"  ]] || WIDTH="$(read_cfg_int "$CONFIG" width 10)"
  [[ -n "$HEIGHT" ]] || HEIGHT="$(read_cfg_int "$CONFIG" height 10)"
  [[ -n "$FREQ"   ]] || FREQ="$(read_cfg_int "$CONFIG" freq 100)"
  [[ -n "$AGENTS" ]] || AGENTS="$(read_cfg_int "$CONFIG" n_agents 6)"
  [[ -n "$ROLE_DIM" ]]  || ROLE_DIM="$(read_cfg_int "$CONFIG" role_dim 6)"
  [[ -n "$INV_EVERY" ]] || INV_EVERY="$(read_cfg_int "$CONFIG" inv_every 10)"

  local PY; PY="$(resolve_python)" || die "no RL python env. Run: scripts/zappy_run.sh setup"
  PYTHONPATH="$AI_DIR" "$PY" -c 'import rl.deploy.zappy_ai_adapter' 2>/dev/null \
    || die "RL deps missing in $PY. Run: scripts/zappy_run.sh setup"

  [[ -n "$OUT_DIR" ]] || OUT_DIR="$ROOT/.runlogs/live-$(date +%Y%m%d-%H%M%S)"
  mkdir -p "$OUT_DIR"

  log "run:    $RUN_DIR"
  log "python: $PY"
  log "map:    ${WIDTH}x${HEIGHT} freq=$FREQ agents=$AGENTS team=$TEAM port=$PORT"
  log "logs:   $OUT_DIR"

  cleanup() {
    local rc=$?
    trap - EXIT INT TERM
    for p in "${AGENT_PIDS[@]:-}"; do kill "$p" 2>/dev/null || true; done
    [[ -n "${GUI_PID:-}" ]] && kill "$GUI_PID" 2>/dev/null || true
    if [[ -n "${SERVER_PID:-}" ]]; then
      if [[ -n "${FIFO:-}" && -e "$FIFO" ]]; then printf '/quit\n' >&3 2>/dev/null || true; sleep 0.3; fi
      kill "$SERVER_PID" 2>/dev/null || true
    fi
    [[ -n "${FIFO:-}" ]] && rm -f "$FIFO"
    exit "$rc"
  }
  trap cleanup EXIT INT TERM

  if [[ "$START_SERVER" == "1" ]]; then
    build_component server "$SERVER_DIR"
    local sbin; sbin="$(resolve_server_bin)" || die "could not build/find zappy_server"
    FIFO="$OUT_DIR/server.stdin"; mkfifo "$FIFO"
    "$sbin" -p "$PORT" -x "$WIDTH" -y "$HEIGHT" -n "$TEAM" -c "$AGENTS" -f "$FREQ" \
      < "$FIFO" > "$OUT_DIR/server.log" 2>&1 &
    SERVER_PID=$!
    exec 3> "$FIFO"
    log "server pid=$SERVER_PID"
    wait_for_port || { log "server did not listen on $HOST:$PORT"; tail -n 40 "$OUT_DIR/server.log" >&2 || true; exit 1; }
  else
    log "attaching to existing server at $HOST:$PORT"
    wait_for_port || die "cannot connect to $HOST:$PORT"
  fi

  if [[ "$START_GUI" == "1" ]]; then
    build_component gui "$GUI_DIR"
    local gbin
    if gbin="$(resolve_gui_bin)"; then
      # run from gui/ so the GUI's relative "assets/..." paths resolve (log path is absolute)
      ( cd "$GUI_DIR" && exec "$gbin" -p "$PORT" -h "$HOST" -v warn --log-file "$OUT_DIR/gui.log" ) \
        > "$OUT_DIR/gui.stdout.log" 2> "$OUT_DIR/gui.stderr.log" &
      GUI_PID=$!
      log "gui pid=$GUI_PID"
    else
      log "GUI unavailable — continuing headless"
    fi
  fi

  local base=( "$PY" -m rl.deploy.zappy_ai_adapter
    --host "$HOST" --port "$PORT" --team "$TEAM"
    --params "$PARAMS" --config "$CONFIG"
    --freq "$FREQ" --inv-every "$INV_EVERY" --role-dim "$ROLE_DIM" )
  [[ "$GREEDY" == "1" ]] && base+=(--greedy)
  [[ "$DURATION" != "0" ]] && base+=(--duration "$DURATION")

  log "launching $AGENTS agents (greedy=$GREEDY duration=$DURATION role_dim=$ROLE_DIM)"
  for ((i = 0; i < AGENTS; i++)); do
    PYTHONPATH="$AI_DIR" "${base[@]}" \
      --role-id "$i" --seed "$i" --report-json "$OUT_DIR/agent-$i.json" \
      > "$OUT_DIR/agent-$i.log" 2>&1 &
    local apid=$!
    AGENT_PIDS+=("$apid")
    log "agent role=$i pid=$apid"
    sleep 0.15
  done

  local fail=0
  set +e
  for p in "${AGENT_PIDS[@]}"; do
    wait "$p"; rc=$?
    [[ "$rc" != "0" && "$rc" != "3" ]] && fail=1
  done
  set -e

  # Summary from per-agent JSON reports.
  python3 - "$OUT_DIR" <<'PY'
import glob, json, os, sys
out = sys.argv[1]
reports = []
for path in sorted(glob.glob(os.path.join(out, "agent-*.json"))):
    try: reports.append(json.load(open(path)))
    except Exception as exc: reports.append({"error": repr(exc), "n_protocol_errors": -1})
print("[zappy] summary")
if not reports:
    print("  no agent reports found"); raise SystemExit(0)
levels = [r.get("final_level") for r in reports]
errors = sum(int(r.get("n_protocol_errors", 0) or 0) for r in reports)
alive = sum(1 for r in reports if r.get("alive"))
print(f"  final_levels={levels}")
print(f"  alive={alive}/{len(reports)} protocol_errors={errors}")
for r in reports:
    print(f"  role {r.get('role_id','?')}: L{r.get('final_level')} cycles={r.get('cycles')} "
          f"incant_ok={r.get('incant_ok')} incant_ko={r.get('incant_ko')} errors={r.get('n_protocol_errors')}")
print(f"  logs={out}")
PY

  # Keep the world up so you can watch/inspect: don't auto-close the GUI when the
  # agents finish — wait until you close the window or press Ctrl+C.
  if [[ "$START_GUI" == "1" && -n "${GUI_PID:-}" ]] && kill -0 "$GUI_PID" 2>/dev/null; then
    log "agents finished — GUI still open. Close the window or press Ctrl+C to stop."
    wait "$GUI_PID" 2>/dev/null || true
  fi

  [[ "$fail" == "0" ]] || exit 1
}

# ---- dispatch --------------------------------------------------------------
main() {
  local cmd="${1:-}"; [[ $# -gt 0 ]] && shift || true
  case "$cmd" in
    setup)        parse_opts "$@"; cmd_setup ;;
    server)       parse_opts "$@"; cmd_server ;;
    gui)          parse_opts "$@"; cmd_gui ;;
    ai)           parse_opts "$@"; cmd_ai ;;
    model)        parse_opts "$@"; cmd_model ;;
    demo)         parse_opts "$@"; START_GUI="1"; cmd_model ;;
    -h|--help|help|"") usage ;;
    *) echo "unknown command: $cmd" >&2; usage >&2; exit 2 ;;
  esac
}
main "$@"
