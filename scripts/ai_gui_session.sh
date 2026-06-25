#!/usr/bin/env bash
# Live session: zappy_server + real zappy_ai clients (from ai/) + visible GUI.
# Watch the AIs play and elevate in the GUI window.
# Usage: ./scripts/ai_gui_session.sh [port] [nb_ai] [map_w] [map_h] [freq] [seconds]
set -e

PORT=${1:-4242}
NB_AI=${2:-6}
MAP_W=${3:-10}
MAP_H=${4:-10}
FREQ=${5:-100}
SECONDS_RUN=${6:-90}

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SERVER="$ROOT/zappy_server"
GUI="$ROOT/gui/zappy_gui"
AI="$ROOT/zappy_ai"
DISPLAY="${DISPLAY:-:1}"
export DISPLAY

# Build anything missing / stale.
echo "[sess] building server + ai..."
( cd "$ROOT" && make zappy_server >/dev/null 2>&1 )
( cd "$ROOT/ai" && make >/dev/null 2>&1 )
[ -x "$GUI" ] || { echo "[sess] GUI not built; run: cd gui && make"; exit 1; }

AI_PIDS=()
cleanup() {
    echo "[sess] stopping..."
    kill "$GUI_PID" 2>/dev/null || true
    for p in "${AI_PIDS[@]}"; do kill "$p" 2>/dev/null || true; done
    kill "$SRV_PID" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

echo "[sess] server: ${MAP_W}x${MAP_H} port=$PORT freq=$FREQ team=team1 slots=$((NB_AI+2))"
"$SERVER" -p "$PORT" -x "$MAP_W" -y "$MAP_H" -n team1 -c "$((NB_AI + 2))" -f "$FREQ" &
SRV_PID=$!
sleep 0.8
kill -0 "$SRV_PID" 2>/dev/null || { echo "[sess] server failed"; exit 1; }

echo "[sess] launching GUI on DISPLAY=$DISPLAY (cwd=gui/ for assets)"
( cd "$ROOT/gui" && exec ./zappy_gui -p "$PORT" ) &
GUI_PID=$!
sleep 2.0
echo "[sess] GUI pid=$GUI_PID cwd=$(readlink /proc/$GUI_PID/cwd 2>/dev/null)"

echo "[sess] starting $NB_AI AI clients (team1)..."
for i in $(seq 1 "$NB_AI"); do
    "$AI" -p "$PORT" -n team1 >/dev/null 2>&1 &
    AI_PIDS+=($!)
    sleep 0.3
done

echo "[sess] running ${SECONDS_RUN}s — watch the GUI. Ctrl+C to stop early."
END=$((SECONDS + SECONDS_RUN))
while [ "$SECONDS" -lt "$END" ]; do
    kill -0 "$SRV_PID" 2>/dev/null || { echo "[sess] server exited"; break; }
    sleep 2
done
echo "[sess] done."
