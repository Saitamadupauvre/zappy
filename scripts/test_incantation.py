#!/usr/bin/env python3
"""Deterministic incantation protocol tests against zappy_server.

Uses a 1x1 map so resource_target's `count<1 -> 1` floor guarantees >=1 of
every resource on the sole tile (0,0), and every player spawns on (0,0).
That makes level 1->2 incantations fully deterministic (1 player + 1 linemate).

Drives raw sockets: one GRAPHIC observer + N player clients. Verifies the
player-facing replies (Elevation underway / Current level / ko) AND the GUI
broadcast triplet (pic / pie / plv) emitted by handle_incantation/finish.
"""
import socket
import subprocess
import sys
import time
import os

SERVER = os.path.join(os.path.dirname(__file__), "..", "zappy_server")
PORT = 4799
FREQ = 1000  # CD_INCANTATION(300)*1000/freq = 300ms per ritual


class Conn:
    def __init__(self, port):
        self.s = socket.create_connection(("127.0.0.1", port))
        self.s.settimeout(3.0)
        self.buf = b""
        self.all = []  # every line ever received on this conn

    def send(self, line):
        self.s.sendall(line.encode() if line.endswith("\n") else (line + "\n").encode())

    def lines(self, timeout=1.0):
        """Drain everything received within `timeout`, return list of lines."""
        out = []
        end = time.time() + timeout
        self.s.settimeout(0.1)
        while time.time() < end:
            try:
                data = self.s.recv(4096)
                if not data:
                    break
                self.buf += data
            except socket.timeout:
                pass
            while b"\n" in self.buf:
                line, self.buf = self.buf.split(b"\n", 1)
                d = line.decode(errors="replace")
                out.append(d)
                self.all.append(d)
        return out

    def close(self):
        try:
            self.s.close()
        except OSError:
            pass


def start_server():
    p = subprocess.Popen(
        [SERVER, "-p", str(PORT), "-x", "1", "-y", "1",
         "-n", "team1", "-c", "10", "-f", str(FREQ)],
        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    time.sleep(0.6)
    if p.poll() is not None:
        sys.exit(f"server died: {p.stderr.read().decode()}")
    return p


PASS, FAIL = "\033[32mPASS\033[0m", "\033[31mFAIL\033[0m"
results = []


def check(name, cond, detail=""):
    results.append(cond)
    print(f"  [{PASS if cond else FAIL}] {name}" + (f"  -- {detail}" if detail and not cond else ""))


def new_player(port):
    """Connect a player, consume welcome (num\\nX Y\\n), return (Conn, fd-unknown)."""
    c = Conn(port)
    c.send("team1")
    c.lines(0.5)  # welcome: remaining-slots + "X Y"
    return c


def test_solo():
    print("Test 1: solo level 1->2 (1 player, 1 linemate)")
    gui = Conn(PORT)
    gui.send("GRAPHIC")
    gui.lines(0.5)  # msz/sgt/mct/etc handshake dump

    p = new_player(PORT)
    gui.lines(0.5)  # pnw for the new player

    p.send("Incantation")
    # ritual = CD_INCANTATION*1000/freq = 300ms; drain past completion
    p.lines(1.2)
    gui.lines(0.4)

    pl, gl = p.all, gui.all
    check("player got 'Elevation underway'", "Elevation underway" in pl, pl)
    check("GUI got pic at 0 0 1", any(x.startswith("pic 0 0 1") for x in gl), gl)
    check("player got 'Current level: 2'", "Current level: 2" in pl, pl)
    check("GUI got pie 0 0 1 (success)", "pie 0 0 1" in gl, [x for x in gl if x.startswith("pie")])
    check("GUI got plv ... 2", any(x.startswith("plv ") and x.endswith(" 2") for x in gl),
          [x for x in gl if x.startswith("plv")])

    p.close()
    gui.close()


def test_dual():
    print("Test 2: two co-located level-1 players both elevate")
    gui = Conn(PORT)
    gui.send("GRAPHIC")
    gui.lines(0.5)

    a = new_player(PORT)
    b = new_player(PORT)
    gui.lines(0.5)

    a.send("Incantation")
    a.lines(1.2)
    b.lines(0.2)
    gui.lines(0.4)

    al, bl, gl = a.all, b.all, gui.all
    check("initiator got 'Elevation underway'", "Elevation underway" in al, al)
    check("2nd player got 'Elevation underway'", "Elevation underway" in bl, bl)
    pic = [x for x in gl if x.startswith("pic 0 0 1")]
    # pic line lists both participant fds (two '#' tokens)
    check("pic lists both participants",
          bool(pic) and pic[-1].count("#") == 2, pic)

    check("initiator reached level 2", "Current level: 2" in al, al)
    check("2nd player reached level 2", "Current level: 2" in bl, bl)
    check("GUI got pie success", "pie 0 0 1" in gl, [x for x in gl if x.startswith("pie")])
    plv = [x for x in gl if x.startswith("plv ")]
    check("GUI got 2 plv updates to lvl 2",
          len([x for x in plv if x.endswith(" 2")]) == 2, plv)

    a.close()
    b.close()
    gui.close()


def main():
    srv = start_server()
    try:
        test_solo()
        test_dual()
    finally:
        srv.terminate()
        try:
            srv.wait(2)
        except subprocess.TimeoutExpired:
            srv.kill()
    print()
    ok = all(results)
    print(f"{sum(results)}/{len(results)} checks passed -> {'ALL PASS' if ok else 'FAILURES'}")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
