#!/usr/bin/env python3
"""Full protocol simulation: exercise every server command + error paths.

Single 1x1 map with 2 teams. Players spawn co-located on (0,0) and every
resource sits on that tile, so commands needing two players on one tile
(Eject, Incantation) and resource ops (Take/Set) are deterministic with no
navigation.

Player commands (12): Forward Right Left Look Inventory Broadcast Connect_nbr
                      Fork Eject Take Set Incantation
GUI commands (10):    msz mct tna sgt bct sst ppo plv pin stu
Error paths:          unknown command, missing parameter (both -> "ko")
"""
import socket
import subprocess
import sys
import time
import os

SERVER = os.path.join(os.path.dirname(__file__), "..", "zappy_server")
PORT = 4793
FREQ = 1000


class Conn:
    def __init__(self, port):
        self.s = socket.create_connection(("127.0.0.1", port))
        self.buf = b""
        self.all = []

    def send(self, line):
        self.s.sendall((line + "\n").encode())

    def lines(self, timeout=0.25):
        end = time.time() + timeout
        self.s.settimeout(0.02)
        out = []
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
            while b"\n" in self.buf:
                ln, self.buf = self.buf.split(b"\n", 1)
                t = ln.decode(errors="replace")
                out.append(t)
                self.all.append(t)
        return out

    def ask(self, cmd, timeout=0.3):
        """Send a command, return the lines that arrive right after."""
        self.lines(0.02)
        self.send(cmd)
        return self.lines(timeout)

    def close(self):
        try:
            self.s.close()
        except OSError:
            pass


PASS, FAIL = "\033[32mPASS\033[0m", "\033[31mFAIL\033[0m"
results = []


def check(name, cond, detail=""):
    results.append(bool(cond))
    mark = PASS if cond else FAIL
    print(f"  [{mark}] {name}" + (f"  -- {detail}" if detail and not cond else ""))


def has(lines, pred):
    return any(pred(x) for x in lines)


def start_server():
    p = subprocess.Popen(
        [SERVER, "-p", str(PORT), "-x", "1", "-y", "1",
         "-n", "team1", "team2", "-c", "10", "-f", str(FREQ)],
        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    time.sleep(0.5)
    if p.poll() is not None:
        sys.exit("server died: " + p.stderr.read().decode())
    return p


def new_player(team):
    c = Conn(PORT)
    c.send(team)
    c.lines(0.3)  # WELCOME / slots / "X Y"
    return c


def main():
    srv = start_server()
    gui = Conn(PORT)
    gui.send("GRAPHIC")
    gui.lines(0.4)

    print("== GUI commands ==")
    check("msz -> 'msz 1 1'", has(gui.ask("msz"), lambda x: x == "msz 1 1"))
    check("sgt -> 'sgt 1000'", has(gui.ask("sgt"), lambda x: x == f"sgt {FREQ}"))
    check("sst 1000 -> 'sst 1000'", has(gui.ask("sst 1000"), lambda x: x.startswith("sst ")))
    tna = gui.ask("tna")
    check("tna -> both teams", has(tna, lambda x: x == "tna team1") and
          has(tna, lambda x: x == "tna team2"), tna)
    check("bct 0 0 -> 'bct 0 0 ...'",
          has(gui.ask("bct 0 0"), lambda x: x.startswith("bct 0 0 ")))
    check("mct -> bct line(s)", has(gui.ask("mct"), lambda x: x.startswith("bct ")))
    check("stu -> uptime number", has(gui.ask("stu"), lambda x: x.strip().isdigit()))
    check("unknown gui cmd -> 'ko'", has(gui.ask("frobnicate"), lambda x: x == "ko"))
    check("bct missing args -> 'ko'", has(gui.ask("bct"), lambda x: x == "ko"))

    print("\n== connect players (team1 x2 on tile 0,0) ==")
    p1 = new_player("team1")
    p2 = new_player("team1")
    pnw = gui.lines(0.3)
    fds = [x.split()[1] for x in gui.all if x.startswith("pnw #")]
    check("GUI got pnw for 2 players", len(fds) >= 2, fds)
    fd1 = fds[0] if fds else "#1"

    print("\n== player info/movement ==")
    inv = p1.ask("Inventory")
    check("Inventory -> '[food N, linemate N, ...]'",
          has(inv, lambda x: x.startswith("[food ") and "thystame" in x and x.endswith("]")), inv)
    look = p1.ask("Look")
    check("Look -> '[...]'", has(look, lambda x: x.startswith("[") and x.endswith("]")), look)
    check("Connect_nbr -> integer", has(p1.ask("Connect_nbr"), lambda x: x.strip().isdigit()))
    time.sleep(0.05)
    check("Forward -> ok", has(p1.ask("Forward"), lambda x: x == "ok"))
    time.sleep(0.05)
    check("Right -> ok", has(p1.ask("Right"), lambda x: x == "ok"))
    time.sleep(0.05)
    check("Left -> ok", has(p1.ask("Left"), lambda x: x == "ok"))

    print("\n== broadcast ==")
    p2.lines(0.05)
    bc = p1.ask("Broadcast hello-world")
    p2r = p2.lines(0.2)
    gui.lines(0.15)  # drain async GUI notifications
    check("Broadcast -> ok", has(bc, lambda x: x == "ok"))
    check("other player got 'message K, hello-world'",
          has(p2r, lambda x: x.startswith("message ") and x.endswith("hello-world")), p2r)
    check("GUI got pbc", has(gui.all, lambda x: x.startswith("pbc ")))

    print("\n== take / set ==")
    time.sleep(0.05)
    tk = p1.ask("Take linemate")
    gui.lines(0.1)
    check("Take linemate -> ok", has(tk, lambda x: x == "ok"), tk)
    check("GUI got pgt (gui take notify)", has(gui.all, lambda x: x.startswith("pgt ")))
    inv2 = p1.ask("Inventory")
    check("Inventory shows linemate 1", has(inv2, lambda x: "linemate 1" in x), inv2)
    time.sleep(0.05)
    st = p1.ask("Set linemate")
    gui.lines(0.1)
    check("Set linemate -> ok", has(st, lambda x: x == "ok"), st)
    check("GUI got pdr (gui drop notify)", has(gui.all, lambda x: x.startswith("pdr ")))

    print("\n== fork ==")
    time.sleep(0.05)
    cn_before = [int(x) for x in p1.ask("Connect_nbr") if x.strip().isdigit()]
    time.sleep(0.05)
    fk = p1.ask("Fork", 0.4)
    gui.lines(0.15)
    check("Fork -> ok", has(fk, lambda x: x == "ok"), fk)
    check("GUI got pfk + enw (egg laid)",
          has(gui.all, lambda x: x.startswith("pfk ")) and
          has(gui.all, lambda x: x.startswith("enw ")))
    time.sleep(0.05)
    cn_after = [int(x) for x in p1.ask("Connect_nbr") if x.strip().isdigit()]
    check("Connect_nbr rose after Fork", cn_before and cn_after and cn_after[0] > cn_before[0],
          f"{cn_before} -> {cn_after}")

    print("\n== eject ==")
    p2.lines(0.05)
    time.sleep(0.05)
    ej = p1.ask("Eject", 0.4)
    p2e = p2.lines(0.2)
    gui.lines(0.15)
    check("Eject -> ok (p2 co-located pushed)", has(ej, lambda x: x == "ok"), ej)
    check("victim got 'eject: K'", has(p2e, lambda x: x.startswith("eject:")), p2e)
    check("GUI got pex", has(gui.all, lambda x: x.startswith("pex ")))

    print("\n== GUI per-player queries ==")
    check(f"ppo {fd1} -> 'ppo {fd1} ...'",
          has(gui.ask(f"ppo {fd1}"), lambda x: x.startswith(f"ppo {fd1} ")))
    check(f"plv {fd1} -> 'plv {fd1} 1'",
          has(gui.ask(f"plv {fd1}"), lambda x: x.startswith(f"plv {fd1} ")))
    check(f"pin {fd1} -> 'pin {fd1} ...'",
          has(gui.ask(f"pin {fd1}"), lambda x: x.startswith(f"pin {fd1} ")))

    print("\n== incantation (last: changes level) ==")
    # ensure both idle so finish's channeling recheck counts both
    time.sleep(0.1)
    p1.lines(0.05); p2.lines(0.05)
    p1.send("Incantation")
    inc1, inc2, g = [], [], []
    end = time.time() + 2.0
    while time.time() < end and not ("Current level: 2" in inc1 and "Current level: 2" in inc2):
        inc1 += p1.lines(0.05)
        inc2 += p2.lines(0.05)
    g = gui.lines(0.2)
    check("initiator: 'Elevation underway'", has(inc1, lambda x: x == "Elevation underway"), inc1)
    check("initiator reached level 2", has(inc1, lambda x: x == "Current level: 2"), inc1)
    check("2nd player reached level 2", has(inc2, lambda x: x == "Current level: 2"), inc2)
    check("GUI got pic + pie + plv->2",
          has(gui.all, lambda x: x.startswith("pic ")) and
          has(gui.all, lambda x: x.startswith("pie ")) and
          has(gui.all, lambda x: x.startswith("plv ") and x.endswith(" 2")))

    print("\n== player error paths ==")
    check("unknown player cmd -> 'ko'", has(p1.ask("Dance"), lambda x: x == "ko"))
    check("Take (no arg) -> 'ko'", has(p1.ask("Take"), lambda x: x == "ko"))

    gui.close(); p1.close(); p2.close()
    srv.terminate()
    try:
        srv.wait(2)
    except subprocess.TimeoutExpired:
        srv.kill()

    print()
    ok = all(results)
    print(f"{sum(results)}/{len(results)} checks passed -> "
          f"{'ALL COMMANDS OK' if ok else 'FAILURES'}")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
