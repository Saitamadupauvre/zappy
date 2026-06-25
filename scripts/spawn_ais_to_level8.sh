#!/usr/bin/env bash
# Spawn enough zappy_ai clients on ONE team to allow elevation up to level 8.
#
# Reaching level 8 needs 6 players together on a tile (incantation requirement
# is highest at levels 6->7 and 7->8: 6 players each). So 6 AIs on the same
# team is the minimum. Server is assumed to already be running.
#
# Usage: ./scripts/spawn_ais_to_level8.sh [port] [team] [host] [nb_ai]
set -u

PORT=${1:-4242}
TEAM=${2:-team1}
HOST=${3:-localhost}
NB_AI=${4:-6}            # >=6 required for L7->L8

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
AI="$ROOT/zappy_ai"

if [[ ! -e "$AI" ]]; then
    echo "[spawn] AI binary not found at $AI" >&2
    exit 1
fi
if (( NB_AI < 6 )); then
    echo "[spawn] warning: NB_AI=$NB_AI < 6 — cannot reach level 8 (needs 6 players)" >&2
fi

PIDS=()
cleanup() {
    echo "[spawn] stopping ${#PIDS[@]} AIs..."
    for pid in "${PIDS[@]}"; do
        kill "$pid" 2>/dev/null
    done
    wait 2>/dev/null
}
trap cleanup INT TERM EXIT

echo "[spawn] launching $NB_AI AIs on team '$TEAM' -> $HOST:$PORT"
for ((i = 0; i < NB_AI; i++)); do
    "$AI" -p "$PORT" -n "$TEAM" -h "$HOST" &
    PIDS+=("$!")
    echo "[spawn]   AI #$((i + 1)) pid=$!"
    sleep 0.2          # stagger so the server registers each connect cleanly
done

echo "[spawn] all AIs running. Ctrl-C to stop."
wait
