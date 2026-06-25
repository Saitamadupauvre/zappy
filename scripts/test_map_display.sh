#!/usr/bin/env bash
# Launch server + N dummy AIs for GUI map display testing.
# Usage: ./scripts/test_map_display.sh [port] [nb_ai] [map_w] [map_h] [freq]

set -e

PORT=${1:-4242}
NB_AI=${2:-50}
MAP_W=${3:-10}
MAP_H=${4:-10}
FREQ=${5:-1}
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SERVER="$ROOT/gui/zappy_server"
AI_SCRIPT="$SCRIPT_DIR/dummy_ai.py"

if [[ ! -x "$SERVER" ]]; then
    echo "[test] server binary not found at $SERVER" >&2
    exit 1
fi

# Kill everything on exit
cleanup() {
    echo "[test] stopping server and AIs..."
    kill "$SERVER_PID" 2>/dev/null || true
    for pid in "${AI_PIDS[@]}"; do
        kill "$pid" 2>/dev/null || true
    done
}
trap cleanup EXIT INT TERM

echo "[test] starting server: ${MAP_W}x${MAP_H} port=$PORT freq=$FREQ"
"$SERVER" -p "$PORT" -x "$MAP_W" -y "$MAP_H" -n team1 team2 team3 team4 team5 team6 team7 team8 maozedon -c 10 -f "$FREQ" &
SERVER_PID=$!

echo "[test] waiting for server..."
sleep 1

# Verify server started
if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "[test] server failed to start" >&2
    exit 1
fi

AI_PIDS=()
for i in $(seq 1 "$NB_AI"); do
    TEAM="team$((i % 8 + 1))"
    DELAY=$(python3 -c "print(0.2 + $i * 0.05)")
    python3 "$AI_SCRIPT" --port "$PORT" --team "$TEAM" --delay "$DELAY" &
    AI_PIDS+=($!)
    echo "[test] started AI #$i (team=$TEAM pid=${AI_PIDS[-1]})"
done

echo ""
echo "================================================================"
echo "  Server running on port $PORT with $NB_AI AI players"
echo "  Run the GUI in another terminal:"
echo ""
echo "    ./zappy_gui -p $PORT"
echo ""
echo "  Press Ctrl+C to stop everything."
echo "================================================================"
echo ""

# Wait for server to exit (or Ctrl+C)
wait "$SERVER_PID"
