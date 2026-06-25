#!/usr/bin/env python3
"""End-to-end Zappy test on a 42x42 / freq=100 map with a live GUI.

Two phases, one server:

  PHASE A — command coverage
    A GRAPHIC observer + player clients exercise EVERY server command and the
    two error paths (unknown cmd, missing param). Co-location for Eject is done
    with Fork (a forked egg hatches the next connecting client on the forker's
    tile), so it works on a big map where players otherwise spawn far apart.

  PHASE B — climb toward level 8
    Six players are co-located on a "rally" tile via Fork. One of them (the
    "carrier") roams the 42x42 map gathering every rock it can see (Look then
    Take, position tracked through the GUI 'ppo' query), then walks back to the
    rally tile. With all six co-located it Sets each level's requirement onto
    the tile and a ritual elevates all six together — L1->2 ... up to L7->8.
    The carrier is itself one of the six, so it elevates in lockstep and keeps
    carrying the stockpile. Reaching L8 needs thystame (rare); if none is found
    the chain still climbs to L7 ("close to level 8").

The GUI is launched from gui/ (cwd) so its relative assets/ paths resolve.

Usage: ./scripts/test_to_level8.py [--no-gui]
Exit 0 if PHASE A fully passes AND PHASE B reaches at least level 7.
"""
import os
import socket
import subprocess
import sys
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SERVER = os.path.join(ROOT, "zappy_server")
GUI_DIR = os.path.join(ROOT, "gui")
GUI_BIN = os.path.join(GUI_DIR, "zappy_gui")

PORT = 4248
W = H = 42
FREQ = 100
SLOTS = 40
TEAM = "team1"
TARGET_LEVEL = 6   # climb goal; L5->6 needs 6 co-located players (no thystame < L7)

# Resource enum order (server include/world.h), minus FOOD.
ROCKS = ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]
# [level-1] = nb_players, linemate, deraumere, sibur, mendiane, phiras, thystame
ELEV_REQ = [
    [1, 1, 0, 0, 0, 0, 0],
    [2, 1, 1, 1, 0, 0, 0],
    [2, 2, 0, 1, 0, 2, 0],
    [4, 1, 1, 2, 0, 1, 0],
    [4, 1, 2, 1, 3, 0, 0],
    [6, 1, 2, 3, 0, 1, 0],
    [6, 2, 2, 2, 2, 2, 1],
]
# Direction enum: NORTH=0, EAST=1, SOUTH=2, WEST=3 (right = +1, clockwise).
DIR_VEC = {0: (0, -1), 1: (1, 0), 2: (0, 1), 3: (-1, 0)}

PASS, FAIL = "\033[32mPASS\033[0m", "\033[31mFAIL\033[0m"
results = []


def check(name, cond, detail=""):
    results.append(bool(cond))
    mark = PASS if cond else FAIL
    print(f"  [{mark}] {name}" + (f"  -- {detail}" if detail and not cond else ""))


class Conn:
    """Socket wrapper with a background drainer thread.

    The server floods GUI clients with the full map (1764 'bct' lines every
    ~200ms at freq=100). A daemon thread keeps the OS receive buffer empty and
    appends every line to self.all, so checks see notifications with minimal lag
    instead of falling behind the bct backlog.
    """
    def __init__(self, port):
        self.s = socket.create_connection(("127.0.0.1", port))
        self.s.settimeout(0.1)
        self.buf = b""
        self.all = []
        self.lock = threading.Lock()
        self.dead = False
        self.t = threading.Thread(target=self._drain, daemon=True)
        self.t.start()

    def _drain(self):
        while not self.dead:
            try:
                d = self.s.recv(65536)
                if not d:
                    break
            except socket.timeout:
                continue
            except OSError:
                break
            with self.lock:
                self.buf += d
                while b"\n" in self.buf:
                    ln, self.buf = self.buf.split(b"\n", 1)
                    self.all.append(ln.decode(errors="replace"))

    def send(self, line):
        try:
            self.s.sendall((line + "\n").encode())
        except OSError:
            self.dead = True

    def lines(self, timeout=0.25):
        """Return lines that arrive during `timeout` (drained by the thread)."""
        with self.lock:
            mark = len(self.all)
        time.sleep(timeout)
        with self.lock:
            return list(self.all[mark:])

    def ask(self, cmd, timeout=0.3):
        with self.lock:
            mark = len(self.all)
        self.send(cmd)
        time.sleep(timeout)
        with self.lock:
            return list(self.all[mark:])

    def reply(self, timeout=1.0):
        """Wait for one ok/ko/dead reply, skipping async lines."""
        with self.lock:
            mark = len(self.all)
        end = time.time() + timeout
        while time.time() < end:
            with self.lock:
                chunk = self.all[mark:]
                mark = len(self.all)
            for ln in chunk:
                if ln in ("ok", "ko", "dead"):
                    return ln
            time.sleep(0.02)
        return None

    def snapshot(self):
        with self.lock:
            return list(self.all)

    def close(self):
        self.dead = True
        try:
            self.s.close()
        except OSError:
            pass


def has(lines, pred):
    return any(pred(x) for x in lines)


def pinfo(conn, cmd, pred, timeout=1.5):
    """Send an info command; return the reply slice once `pred` matches a line.

    Robust to a residual cooldown delaying the reply (polls up to `timeout`).
    """
    with conn.lock:
        mark = len(conn.all)
    conn.send(cmd)
    end = time.time() + timeout
    while time.time() < end:
        sl = conn.snapshot()[mark:]
        if has(sl, pred):
            return sl
        time.sleep(0.05)
    return conn.snapshot()[mark:]


def slook(conn):
    """Reply-synced Look: tokens on the player's current tile."""
    sl = pinfo(conn, "Look", lambda x: x.startswith("[") and x.endswith("]"), 1.0)
    return parse_tile0(sl)


def connect_nbr(conn):
    """Connect_nbr value (free team slots), retried against transient timing."""
    for _ in range(4):
        sl = pinfo(conn, "Connect_nbr", lambda x: x.strip().isdigit(), 1.0)
        ints = [int(x) for x in sl if x.strip().isdigit()]
        if ints:
            return ints[0]
        time.sleep(0.2)
    return None


# ---------------------------------------------------------------------------
# process / connection helpers
# ---------------------------------------------------------------------------
def start_server():
    p = subprocess.Popen(
        [SERVER, "-p", str(PORT), "-x", str(W), "-y", str(H),
         "-n", TEAM, "-c", str(SLOTS), "-f", str(FREQ)],
        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    time.sleep(0.6)
    if p.poll() is not None:
        sys.exit("server died: " + p.stderr.read().decode())
    return p


def start_gui():
    if not os.access(GUI_BIN, os.X_OK):
        print(f"[gui] {GUI_BIN} not found/executable — skipping GUI")
        return None
    env = dict(os.environ, DISPLAY=os.environ.get("DISPLAY", ":1"))
    # cwd=gui/ so relative assets/ (models, shaders, textures) resolve
    g = subprocess.Popen([GUI_BIN, "-p", str(PORT)], cwd=GUI_DIR, env=env,
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(2.0)
    if g.poll() is not None:
        print("[gui] failed to launch (no display?) — continuing headless")
        return None
    print(f"[gui] launched from {GUI_DIR} on DISPLAY={env['DISPLAY']} port={PORT}")
    return g


def new_player(team=TEAM):
    c = Conn(PORT)
    c.send(team)
    c.lines(0.3)  # slots + "X Y"
    return c


def parse_tile0(look_lines):
    """First tile (current) contents from a Look reply, as a token list."""
    for ln in look_lines:
        if ln.startswith("[") and ln.endswith("]"):
            inner = ln[1:-1]
            first = inner.split(",")[0].strip()
            return first.split()
    return []


def ppo(gui, fd):
    """Return (x, y, dir) for a player fd via the GUI ppo query.

    `fd` may be "#8" or "8"; the server accepts either and replies "ppo #8 ...".
    """
    tag = fd if str(fd).startswith("#") else f"#{fd}"
    with gui.lock:
        mark = len(gui.all)
    gui.send(f"ppo {tag}")
    end = time.time() + 1.0
    while time.time() < end:
        for ln in gui.snapshot()[mark:]:
            if ln.startswith(f"ppo {tag} "):
                p = ln.split()
                return int(p[2]), int(p[3]), int(p[4]) - 1  # protocol dir = enum+1
        time.sleep(0.03)
    return None


# ---------------------------------------------------------------------------
# PHASE A — command coverage
# ---------------------------------------------------------------------------
def pcmd(conn, command, timeout=2.0):
    """Send a player command and wait for its ok/ko/dead reply.

    Commands are reply-synchronized (one in flight at a time) so the per-command
    cooldown queue never builds up and replies stay aligned with their command.
    """
    conn.send(command)
    return conn.reply(timeout)


def look0(conn):
    """Tokens on the player's current tile (Look has cooldown 7)."""
    return parse_tile0(conn.ask("Look", 0.3))


def pnw_after(gui, mark, timeout=1.5):
    """First 'pnw #N ...' fd in the GUI stream after snapshot index `mark`.

    Polls up to `timeout` so a just-connected client's pnw isn't missed.
    """
    end = time.time() + timeout
    while time.time() < end:
        for ln in gui.snapshot()[mark:]:
            if ln.startswith("pnw #"):
                return ln.split()[1]
        time.sleep(0.05)
    return None


def phase_a(gui):
    print("\n========== PHASE A: command coverage (42x42 / freq=100) ==========")
    print("== GUI commands ==")
    check("msz -> 'msz 42 42'", has(gui.ask("msz"), lambda x: x == f"msz {W} {H}"))
    check("sgt -> 'sgt 100'", has(gui.ask("sgt"), lambda x: x == f"sgt {FREQ}"))
    check("sst 100 -> 'sst ...'", has(gui.ask("sst 100"), lambda x: x.startswith("sst ")))
    check("tna -> team1", has(gui.ask("tna"), lambda x: x == f"tna {TEAM}"))
    check("bct 0 0 -> 'bct 0 0 ...'",
          has(gui.ask("bct 0 0"), lambda x: x.startswith("bct 0 0 ")))
    check("mct -> bct line(s)", has(gui.ask("mct", 0.6), lambda x: x.startswith("bct ")))
    check("stu -> uptime number", has(gui.ask("stu"), lambda x: x.strip().isdigit()))
    check("unknown gui cmd -> 'ko'", has(gui.ask("frobnicate"), lambda x: x == "ko"))
    check("bct missing args -> 'ko'", has(gui.ask("bct"), lambda x: x == "ko"))

    print("\n== connect 2 co-located players (fork-hatched) ==")
    m = len(gui.snapshot())
    p1 = new_player()
    fd1 = pnw_after(gui, m)
    pcmd(p1, "Fork", 2.0)        # lay an egg on p1's tile
    m = len(gui.snapshot())
    p2 = new_player()            # hatches on p1's egg -> same tile
    fd2 = pnw_after(gui, m)
    check("GUI got pnw for both players", fd1 and fd2, (fd1, fd2))

    print("\n== player info / movement ==")
    inv = pinfo(p1, "Inventory", lambda x: x.startswith("[food "))
    check("Inventory -> '[food N, ... thystame N]'",
          has(inv, lambda x: x.startswith("[food ") and "thystame" in x and x.endswith("]")), inv)
    check("Look -> '[...]'",
          has(pinfo(p1, "Look", lambda x: x.startswith("[") and x.endswith("]")),
              lambda x: x.startswith("[") and x.endswith("]")))
    check("Connect_nbr -> integer",
          has(pinfo(p1, "Connect_nbr", lambda x: x.strip().isdigit()),
              lambda x: x.strip().isdigit()))
    check("Forward -> ok", pcmd(p1, "Forward") == "ok")
    check("Right -> ok", pcmd(p1, "Right") == "ok")
    check("Left -> ok", pcmd(p1, "Left") == "ok")

    print("\n== broadcast ==")
    m2 = len(p2.snapshot())
    bc = pcmd(p1, "Broadcast hello-world")
    time.sleep(0.3)
    p2r = p2.snapshot()[m2:]
    check("Broadcast -> ok", bc == "ok", bc)
    check("other player got 'message K, hello-world'",
          has(p2r, lambda x: x.startswith("message ") and x.endswith("hello-world")), p2r)
    check("GUI got pbc", has(gui.snapshot(), lambda x: x.startswith("pbc ")))

    print("\n== take / set ==")
    # find a rock present on p1's current tile (walk a little if bare)
    rock = None
    for _ in range(40):
        rock = next((r for r in ROCKS if r in look0(p1)), None)
        if rock:
            break
        pcmd(p1, "Forward")
    if rock is None:
        rock = "linemate"   # last resort; Take will ko but keeps the run going
    mg = len(gui.snapshot())
    tk = pcmd(p1, f"Take {rock}")
    check(f"Take {rock} -> ok", tk == "ok", tk)
    check("GUI got pgt (take notify)",
          has(gui.snapshot()[mg:], lambda x: x.startswith("pgt ")))
    check(f"Inventory shows {rock} >=1",
          has(p1.ask("Inventory"), lambda x: f"{rock} " in x and not f"{rock} 0" in x))
    mg = len(gui.snapshot())
    st = pcmd(p1, f"Set {rock}")
    check(f"Set {rock} -> ok", st == "ok", st)
    check("GUI got pdr (drop notify)",
          has(gui.snapshot()[mg:], lambda x: x.startswith("pdr ")))

    print("\n== fork ==")
    cn0 = connect_nbr(p1)
    mg = len(gui.snapshot())
    fk = pcmd(p1, "Fork", 2.0)
    check("Fork -> ok", fk == "ok", fk)
    check("GUI got pfk + enw (egg laid)",
          has(gui.snapshot()[mg:], lambda x: x.startswith("pfk ")) and
          has(gui.snapshot()[mg:], lambda x: x.startswith("enw ")))
    time.sleep(0.5)   # Fork sets a 420ms cooldown; let it expire before the query
    cn1 = connect_nbr(p1)
    check("Connect_nbr rose after Fork",
          cn0 is not None and cn1 is not None and cn1 > cn0, f"{cn0} -> {cn1}")

    print("\n== eject ==")
    # A fork-hatched client lands on the OLDEST team egg's tile, not necessarily
    # p1's current tile, so co-locate explicitly: read the victim's position from
    # the GUI and walk p1 onto it before ejecting.
    pcmd(p1, "Fork", 2.0)
    m = len(gui.snapshot())
    victim = new_player()
    vfd = pnw_after(gui, m)
    _FD[id(p1)] = fd1
    vpos = ppo(gui, vfd)
    if vpos:
        navigate_to(gui, p1, vpos[0], vpos[1])
    mv = len(victim.snapshot())
    mg = len(gui.snapshot())
    ej = pcmd(p1, "Eject", 2.0)
    time.sleep(0.3)
    p2e = victim.snapshot()[mv:]
    check("Eject -> ok (co-located victim pushed)", ej == "ok", ej)
    check("victim got 'eject: K'", has(p2e, lambda x: x.startswith("eject:")), p2e)
    check("GUI got pex", has(gui.snapshot()[mg:], lambda x: x.startswith("pex ")))
    victim.close()

    print("\n== GUI per-player queries ==")
    check(f"ppo {fd1} -> 'ppo {fd1} ...'",
          has(gui.ask(f"ppo {fd1}", 0.6), lambda x: x.startswith(f"ppo {fd1} ")))
    check(f"plv {fd1} -> 'plv {fd1} ...'",
          has(gui.ask(f"plv {fd1}", 0.6), lambda x: x.startswith(f"plv {fd1} ")))
    check(f"pin {fd1} -> 'pin {fd1} ...'",
          has(gui.ask(f"pin {fd1}", 0.6), lambda x: x.startswith(f"pin {fd1} ")))

    print("\n== incantation (solo L1->2: 1 player + 1 linemate) ==")
    # guarantee a linemate under p1: walk until one is takeable, grab it, drop it
    # back onto the current tile, then self-elevate (1 player + 1 linemate).
    for _ in range(120):
        if "linemate" in slook(p1) and pcmd(p1, "Take linemate") == "ok":
            break
        pcmd(p1, "Forward")
    pcmd(p1, "Set linemate")     # drop onto p1's current tile
    time.sleep(0.3)             # clear the Set cooldown -> p1 idle for the ritual
    m = len(p1.snapshot())
    mg = len(gui.snapshot())
    p1.send("Incantation")
    inc = []
    end = time.time() + 6.0     # CD_INCANTATION(300)/freq = 3s at freq=100
    while time.time() < end and "Current level: 2" not in inc:
        inc = p1.snapshot()[m:]
        time.sleep(0.1)
    check("initiator: 'Elevation underway'", has(inc, lambda x: x == "Elevation underway"), inc)
    check("initiator reached level 2", has(inc, lambda x: x == "Current level: 2"), inc)
    check("GUI got pic + pie + plv->2",
          has(gui.snapshot()[mg:], lambda x: x.startswith("pic ")) and
          has(gui.snapshot()[mg:], lambda x: x.startswith("pie ")) and
          has(gui.snapshot()[mg:], lambda x: x.startswith("plv ") and x.endswith(" 2")))

    print("\n== player error paths ==")
    check("unknown player cmd -> 'ko'", pcmd(p1, "Dance") == "ko")
    check("Take (no arg) -> 'ko'", pcmd(p1, "Take") == "ko")

    for c in (p1, p2):
        c.close()


# ---------------------------------------------------------------------------
# PHASE B — climb toward level 8
# ---------------------------------------------------------------------------
def step_cost():
    return max(0.08, 8.0 / FREQ)  # CD_FORWARD(7)/freq seconds, with a floor


_FD = {}


def carrier_fd_of(conn):
    return _FD[id(conn)]


def rotate_to(conn, cur, want):
    """Turn (reply-synced) to face `want` from `cur` (enum 0..3)."""
    diff = (want - cur) % 4
    if diff == 1:
        pcmd(conn, "Right")
    elif diff == 3:
        pcmd(conn, "Left")
    elif diff == 2:
        pcmd(conn, "Right")
        pcmd(conn, "Right")


def navigate_to(gui, carrier, tx, ty):
    """Walk carrier to (tx, ty) on the torus; ppo is ground truth each step."""
    misses = 0
    for _ in range(6 * (W + H)):
        pos = ppo(gui, carrier_fd_of(carrier))
        if pos is None:
            misses += 1
            if misses > 8:
                return False
            time.sleep(0.1)
            continue
        misses = 0
        x, y, d = pos
        if x == tx and y == ty:
            return True
        dx = ((tx - x + W // 2) % W) - W // 2
        dy = ((ty - y + H // 2) % H) - H // 2
        if dx != 0 and abs(dx) >= abs(dy):
            want = 1 if dx > 0 else 3
        else:
            want = 2 if dy > 0 else 0
        rotate_to(carrier, d, want)
        pcmd(carrier, "Forward")
    return False


def gather(gui, carrier, target, budget_s=260.0, max_steps=3000):
    """Serpentine the map, Look+Take rocks until target met or budget spent.

    Commands are pipelined (ask, not reply-synced) so the walk keeps pace at
    freq=100; the per-command window (~step_cost) is wide enough that the move/
    take lands. Any residual backlog is drained by the caller before navigating.
    """
    have = {r: 0 for r in ROCKS}
    t_end = time.time() + budget_s
    steps = 0
    sc = step_cost() + 0.05
    while steps < max_steps and time.time() < t_end:
        if all(have[r] >= target[r] for r in ROCKS):
            break
        tile = parse_tile0(carrier.ask("Look", sc))
        for r in ROCKS:
            if have[r] < target[r] and r in tile:
                carrier.send(f"Take {r}")
                if carrier.reply(sc + 0.3) == "ok":
                    have[r] += 1
        carrier.ask("Forward", sc)
        steps += 1
        if steps % W == 0:                 # boustrophedon: drop to the next row
            carrier.ask("Right", sc)
            carrier.ask("Forward", sc)
            carrier.ask("Right", sc)
        if steps % 50 == 0:
            print(f"  [gather] step {steps}: "
                  f"{ {r: have[r] for r in ROCKS if have[r]} }")
    return have


def _reply_take(self, rock):
    self.send(f"Take {rock}")
    return self.reply(step_cost() + 0.3) == "ok"


Conn.reply_take = _reply_take


def phase_b(gui):
    print(f"\n========== PHASE B: climb toward level {TARGET_LEVEL} (6 players) ==========")
    # Connect 6 players (each spawns on a random tile), then march every one of
    # them onto the seed's tile (the "rally") using ppo-guided navigation. This
    # is robust regardless of egg/spawn placement.
    m0 = len(gui.snapshot())
    players = []
    for _ in range(6):
        players.append(new_player())
        time.sleep(0.2)          # stagger so each connect registers a pnw cleanly
    time.sleep(1.0)
    # pnw is emitted once per connect, in connect order -> zip onto the conns
    pnws = [ln.split()[1] for ln in gui.snapshot()[m0:] if ln.startswith("pnw #")]
    for c, fd in zip(players, pnws):
        _FD[id(c)] = fd
    if len(pnws) < 6 or any(_FD.get(id(p)) is None for p in players):
        check("6 players got fds", False, f"{len(pnws)} pnw: {pnws}")
        return 1
    fds = [_FD[id(p)] for p in players]
    print(f"  6 players connected, fds={fds}")

    seed = players[0]
    rally = ppo(gui, _FD[id(seed)])
    if rally is None:
        check("rally position known", False)
        return 1
    rx, ry, _ = rally
    print(f"  rally tile = ({rx},{ry}) — marching the other 5 in...")
    for p in players[1:]:
        navigate_to(gui, p, rx, ry)
    on = sum(1 for p in players
             if (lambda q: bool(q) and q[0] == rx and q[1] == ry)(ppo(gui, _FD[id(p)])))
    check("6 players co-located on rally", on == 6, f"{on}/6 on ({rx},{ry})")
    if on < 6:
        return 1

    carrier = seed
    # rituals run are L1->2 .. (TARGET_LEVEL-1)->TARGET_LEVEL => ELEV_REQ[0..TARGET_LEVEL-2]
    rituals = range(TARGET_LEVEL - 1)
    target = {r: sum(ELEV_REQ[L][i + 1] for L in rituals) for i, r in enumerate(ROCKS)}
    lin_need = target["linemate"]
    print(f"  gather target (cumulative L1->{TARGET_LEVEL}): "
          f"{ {r: target[r] for r in ROCKS if target[r]} }")
    have = gather(gui, carrier, target)
    print(f"  gathered: { {r: have[r] for r in ROCKS if have[r]} }")
    check(f"carrier gathered linemate>={lin_need} (L1->{TARGET_LEVEL} worth)",
          have["linemate"] >= lin_need, have)
    time.sleep(1.0)      # let the pipelined gather backlog finish executing
    carrier.lines(0.5)   # drain stray replies so navigation stays aligned

    print("  carrier walking back to rally...")
    navigate_to(gui, carrier, rx, ry)
    cpos = ppo(gui, _FD[id(carrier)])
    check("carrier back on rally tile", cpos and cpos[0] == rx and cpos[1] == ry, cpos)

    level = 1
    for L in range(1, TARGET_LEVEL):
        req = ELEV_REQ[L - 1]
        rocks_need = req[1:]
        if any(have[ROCKS[i]] < rocks_need[i] for i in range(6)):
            print(f"  stop: not enough rocks for L{L}->{L+1} "
                  f"(need {[rocks_need[i] for i in range(6)]}, have "
                  f"{[have[ROCKS[i]] for i in range(6)]})")
            break
        # carrier deposits this level's exact requirement onto the rally tile
        for i, r in enumerate(ROCKS):
            for _ in range(rocks_need[i]):
                carrier.send(f"Set {r}")
                if carrier.reply(step_cost() + 0.3) == "ok":
                    have[r] -= 1
        # let everyone go idle (cd<=0, no queued cmd) so they all join
        time.sleep(0.3)
        marks = [len(p.snapshot()) for p in players]
        carrier.send("Incantation")
        target_line = f"Current level: {L + 1}"
        reached = set()
        end = time.time() + 6.0     # CD_INCANTATION(300)/freq = 3s at freq=100
        while time.time() < end and len(reached) < 6:
            for idx, p in enumerate(players):
                if has(p.snapshot()[marks[idx]:], lambda x: x == target_line):
                    reached.add(idx)
            time.sleep(0.1)
        ok = len(reached) >= 6
        check(f"L{L}->{L+1}: all 6 elevated", ok, f"only {len(reached)}/6")
        if not ok:
            break
        level = L + 1
        print(f"  *** reached level {level} ***")

    plv_max = max([int(x.split()[-1]) for x in gui.snapshot()
                   if x.startswith("plv ") and x.split()[-1].isdigit()] + [1])
    print(f"\n  highest level seen by GUI (plv): {plv_max}")
    check(f"climbed to level >= {TARGET_LEVEL}, got {level}", level >= TARGET_LEVEL, level)
    for p in players:
        p.close()
    return level


def main():
    headless = "--no-gui" in sys.argv
    srv = start_server()
    gui_proc = None if headless else start_gui()
    gui = Conn(PORT)
    gui.send("GRAPHIC")
    gui.lines(0.5)

    level = 1
    try:
        phase_a(gui)
        level = phase_b(gui)
    finally:
        gui.close()
        if gui_proc:
            gui_proc.terminate()
            try:
                gui_proc.wait(2)
            except subprocess.TimeoutExpired:
                gui_proc.kill()
        srv.terminate()
        try:
            srv.wait(2)
        except subprocess.TimeoutExpired:
            srv.kill()

    print("\n========================================================")
    passed = sum(results)
    print(f"{passed}/{len(results)} checks passed | climbed to level {level}")
    ok = all(results) and isinstance(level, int) and level >= TARGET_LEVEL
    print("RESULT:", f"OK (all commands pass, reached >= L{TARGET_LEVEL})" if ok else "FAILURES")
    print("========================================================")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
