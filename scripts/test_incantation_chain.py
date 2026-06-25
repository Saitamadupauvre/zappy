#!/usr/bin/env python3
"""Drive a full incantation chain L1 -> L6 on a 1x1 map.

1x1 map keeps all players + all resources on the sole tile (0,0): no
navigation. The density floor (count<1 -> 1) caps each resource at 1 per
refill, so multi-count requirements (e.g. mendiane x3 for L5->6) are met by
having one farmer Take across refill cycles to stockpile in its inventory,
then Set the level's requirement onto the tile right before each ritual.

Players starve (10 food = 1260 game-units < 5*300 incantation units), so every
player tops up food from the tile between rituals.

CRITICAL: all participants must be idle (no queued cmd, cd<=0, ALIVE) at the
moment Incantation is issued, else finish_incantation's channeling_only
recheck counts fewer than required participants and the elevation fails (ko).
"""
import socket
import subprocess
import sys
import time
import os

SERVER = os.path.join(os.path.dirname(__file__), "..", "zappy_server")
PORT = 4795
FREQ = 1000
REFILL_MS = 20_000 / FREQ / 1000.0  # RESPAWN_INTERVAL(20)*1000/freq, in seconds

# [level-1] = {nb_players, linemate, deraumere, sibur, mendiane, phiras, thystame}
ELEV_REQ = [
    [1, 1, 0, 0, 0, 0, 0],
    [2, 1, 1, 1, 0, 0, 0],
    [2, 2, 0, 1, 0, 2, 0],
    [4, 1, 1, 2, 0, 1, 0],
    [4, 1, 2, 1, 3, 0, 0],
    [6, 1, 2, 3, 0, 1, 0],
]
ROCKS = ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]

WATCH_PAUSE = 0.0  # seconds to linger between levels (set >0 for live GUI watching)


class Conn:
    def __init__(self, port):
        self.s = socket.create_connection(("127.0.0.1", port))
        self.s.settimeout(2.0)
        self.buf = b""
        self.all = []

    def send(self, line):
        self.s.sendall((line + "\n").encode())

    def _pump(self, timeout):
        end = time.time() + timeout
        self.s.settimeout(0.02)
        while time.time() < end:
            try:
                d = self.s.recv(4096)
                if not d:
                    break
                self.buf += d
            except socket.timeout:
                pass
            except OSError:
                break

    def lines(self, timeout=0.3):
        self._pump(timeout)
        out = []
        while b"\n" in self.buf:
            line, self.buf = self.buf.split(b"\n", 1)
            d = line.decode(errors="replace")
            out.append(d)
            self.all.append(d)
        return out

    def reply(self, timeout=0.4):
        """Wait for one ok/ko reply, ignoring async lines."""
        end = time.time() + timeout
        while time.time() < end:
            for ln in self.lines(0.05):
                if ln in ("ok", "ko"):
                    return ln
                if ln == "dead":
                    return "dead"
        return None

    def close(self):
        try:
            self.s.close()
        except OSError:
            pass


def start_server():
    p = subprocess.Popen(
        [SERVER, "-p", str(PORT), "-x", "1", "-y", "1",
         "-n", "team1", "-c", "20", "-f", str(FREQ)],
        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    time.sleep(0.5)
    if p.poll() is not None:
        sys.exit("server died: " + p.stderr.read().decode())
    return p


def new_player(port):
    c = Conn(port)
    c.send("team1")
    c.lines(0.3)  # welcome
    return c


def feed(players, rounds):
    """Each player Takes food `rounds` times (skips ko when tile empty)."""
    for _ in range(rounds):
        for p in players:
            p.send("Take food")
        for p in players:
            p.reply()
        time.sleep(REFILL_MS)  # let tile food refill before next grab


def farm_rocks(farmer, need):
    """Ensure farmer inventory holds >= need[i] of each rock (idx into ROCKS)."""
    have = [0] * len(ROCKS)
    deadline = time.time() + 8.0
    while any(have[i] < need[i] for i in range(len(ROCKS))) and time.time() < deadline:
        for i, r in enumerate(ROCKS):
            if have[i] < need[i]:
                farmer.send("Take " + r)
                if farmer.reply() == "ok":
                    have[i] += 1
        time.sleep(REFILL_MS)  # tile refills 1 of each depleted rock
    return have


def drain_idle(players, gui):
    """Flush async traffic and let cooldowns expire so all players are idle."""
    time.sleep(0.05)
    for p in players:
        p.lines(0.05)
    gui.lines(0.05)


PASS, FAIL = "\033[32mPASS\033[0m", "\033[31mFAIL\033[0m"
results = []


def check(name, cond, detail=""):
    results.append(cond)
    mark = PASS if cond else FAIL
    print(f"  [{mark}] {name}" + (f"  -- {detail}" if detail and not cond else ""))


def run_chain():
    gui = Conn(PORT)
    gui.send("GRAPHIC")
    gui.lines(0.4)

    players = [new_player(PORT) for _ in range(4)]
    farmer = players[0]
    gui.lines(0.3)

    print(f"4 players connected on 1x1 map (freq={FREQ}). Driving L1 -> L6.\n")

    # Big food buffer up front so nobody starves across 5 rituals.
    feed(players, 14)

    for L in range(1, 6):
        if WATCH_PAUSE:
            feed(players, 1)  # keep alive while lingering for the viewer
            time.sleep(WATCH_PAUSE)
        req = ELEV_REQ[L - 1]
        rocks_need = req[1:]
        print(f"Level {L} -> {L + 1}: need rocks "
              f"{ {ROCKS[i]: rocks_need[i] for i in range(6) if rocks_need[i]} }")

        # 1) farmer stockpiles the rocks this ritual consumes
        got = farm_rocks(farmer, rocks_need)
        check(f"L{L}: farmed rocks {got[:5]}",
              all(got[i] >= rocks_need[i] for i in range(6)), got)

        # 2) farmer deposits exact requirement onto the tile
        for i, r in enumerate(ROCKS):
            for _ in range(rocks_need[i]):
                farmer.send("Set " + r)
                farmer.reply()

        # 3) keep everyone fed, then make sure all 4 are idle
        feed(players, 2)
        drain_idle(players, gui)
        time.sleep(0.05)  # cd (7ms) well expired -> can_join true for all

        # 4) ritual
        farmer.send("Incantation")
        # collect until every player reports the new level (or timeout)
        target = f"Current level: {L + 1}"
        reached = set()
        end = time.time() + 2.0
        while time.time() < end and len(reached) < 4:
            for idx, p in enumerate(players):
                for ln in p.lines(0.05):
                    if ln == target:
                        reached.add(idx)
        gui.lines(0.2)
        check(f"L{L}->{L+1}: all 4 players elevated", len(reached) == 4,
              f"only {len(reached)}/4 -- last farmer lines: {farmer.all[-4:]}")
        if len(reached) < 4:
            return gui, players  # chain broke, stop early

    # final verification via GUI plv stream: someone hit level 6
    plv6 = [x for x in gui.all if x.startswith("plv ") and x.endswith(" 6")]
    check("GUI saw plv ... 6 (a player reached level 6)", len(plv6) >= 4, plv6[-6:])
    return gui, players


def main():
    srv = start_server()
    try:
        gui, players = run_chain()
        gui.close()
        for p in players:
            p.close()
    finally:
        srv.terminate()
        try:
            srv.wait(2)
        except subprocess.TimeoutExpired:
            srv.kill()
    print()
    ok = all(results)
    print(f"{sum(results)}/{len(results)} checks passed -> "
          f"{'REACHED LEVEL 6' if ok else 'CHAIN FAILED'}")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
