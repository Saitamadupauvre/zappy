#!/usr/bin/env bash
# Build zappy_ai, copy to gui/, then launch server + AI for single-team dev session.
# Usage: ./scripts/dev_session.sh [port] [nb_ai] [map_w] [map_h] [freq]

set -e

PORT=${1:-4242}
NB_AI=${2:-5}
MAP_W=${3:-10}
MAP_H=${4:-10}
FREQ=${5:-5}

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
AI_DIR="$ROOT/ai"
GUI_DIR="$ROOT/gui"
AI_BIN="$GUI_DIR/zappy_ai"
SERVER="$ROOT/zappy_server"

# Build ai
echo "[dev] building zappy_ai..."
(cd "$AI_DIR" && make fclean all)
# zipapp outputs to ROOT (../zappy_ai from ai/), move to gui/
if [[ -f "$ROOT/zappy_ai" ]]; then
    mv "$ROOT/zappy_ai" "$AI_BIN"
elif [[ ! -f "$AI_BIN" ]]; then
    echo "[dev] zappy_ai not found after build" >&2
    exit 1
fi
chmod +x "$AI_BIN"
echo "[dev] zappy_ai ready at $AI_BIN"

if [[ ! -x "$SERVER" ]]; then
    echo "[dev] server binary not found at $SERVER" >&2
    exit 1
fi

cleanup() {
    echo "[dev] stopping..."
    kill "$SERVER_PID" 2>/dev/null || true
    for pid in "${AI_PIDS[@]}"; do
        kill "$pid" 2>/dev/null || true
    done
}
trap cleanup EXIT INT TERM

# server capacity = team_count * client_count; GUI also consumes a slot
# pass NB_AI+1 as -c so there's always room for the GUI to connect
SLOTS=$((NB_AI + 1))
echo "[dev] starting server: ${MAP_W}x${MAP_H} port=$PORT freq=$FREQ team=team1 slots=$SLOTS"
"$SERVER" -p "$PORT" -x "$MAP_W" -y "$MAP_H" -n team1 -c "$SLOTS" -f "$FREQ" &
SERVER_PID=$!

echo "[dev] waiting for server..."
sleep 1

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "[dev] server failed to start" >&2
    exit 1
fi

AI_PIDS=()
for i in $(seq 1 "$NB_AI"); do
    "$AI_BIN" -p "$PORT" -n team1 &
    AI_PIDS+=($!)
    echo "[dev] started AI #$i (pid=${AI_PIDS[-1]})"
done

echo ""
echo "================================================================"
echo "  Server on port $PORT | $NB_AI AIs | team: team1"
echo "  Launch GUI:"
echo "    ./zappy_gui -p $PORT"
echo "  Press Ctrl+C to stop."
echo "================================================================"
echo ""

wait "$SERVER_PID"
