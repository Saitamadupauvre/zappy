#!/usr/bin/env python3
"""Regression test for periodic resource respawn (issue #136).

Starts the server itself and validates, over a GUI (GRAPHIC) connection:
  1. Initial map totals match the spec density targets (refactored refill).
  2. The server broadcasts the full map to GUIs periodically (the 20-tick hook).
  3. On a 1x1 map, a depleted resource is topped back up to its target.

Usage: ./test_respawn.py [path-to-zappy_server]
Exit code 0 on success, 1 on failure.
"""

import os
import socket
import subprocess
import sys
import time

HOST = "127.0.0.1"
SERVER = sys.argv[1] if len(sys.argv) > 1 else "./zappy_server"

# Must mirror densities[] in src/components/world/world.c
DENSITIES = [0.5, 0.3, 0.15, 0.1, 0.1, 0.08, 0.05]
NAMES = ["food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]


def target(w, h, i):
    return max(1, int(w * h * DENSITIES[i]))


def recvlines(sock, duration):
    """Collect newline-terminated lines arriving within `duration` seconds."""
    sock.settimeout(0.1)
    lines, buf, deadline = [], b"", time.time() + duration
    while time.time() < deadline:
        try:
            chunk = sock.recv(4096)
        except socket.timeout:
            continue
        if not chunk:
            break
        buf += chunk
        while b"\n" in buf:
            line, buf = buf.split(b"\n", 1)
            lines.append(line.decode().strip())
    return lines


def gui_connect(port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, port))
    s.settimeout(2)
    assert s.recv(64).startswith(b"WELCOME"), "missing WELCOME"
    s.sendall(b"GRAPHIC\n")
    time.sleep(0.2)
    s.recv(64)  # "GRAPHIC\n" + "W H\n"
    return s


def parse_bct(lines):
    """Return {(x, y): [7 resource counts]} from the latest bct per tile."""
    tiles = {}
    for ln in lines:
        p = ln.split()
        if len(p) == 10 and p[0] == "bct":
            tiles[(int(p[1]), int(p[2]))] = [int(v) for v in p[3:10]]
    return tiles


def start_server(port, w, h, freq):
    proc = subprocess.Popen(
        [SERVER, "-p", str(port), "-x", str(w), "-y", str(h),
         "-n", "team1", "team2", "-c", "5", "-f", str(freq)],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(1)
    return proc


def check_initial_totals(port, w, h):
    s = gui_connect(port)
    s.sendall(b"mct\n")
    tiles = parse_bct(recvlines(s, 0.6))
    s.close()
    assert len(tiles) == w * h, f"got {len(tiles)} tiles, want {w * h}"
    for i in range(7):
        total = sum(t[i] for t in tiles.values())
        want = target(w, h, i)
        assert total == want, f"{NAMES[i]}: total {total} != target {want}"
    print(f"  [ok] initial totals match targets on {w}x{h} map")


def check_periodic_broadcast(port, w, h, freq):
    s = gui_connect(port)
    # Send nothing; the periodic respawn hook should push full-map bct bursts.
    interval = max(0.001, 20.0 / freq)
    lines = recvlines(s, interval * 3)
    s.close()
    bursts = sum(1 for ln in lines if ln.startswith("bct ")) // (w * h)
    assert bursts >= 2, f"only {bursts} full-map broadcasts in 3 intervals"
    print(f"  [ok] received >= 2 periodic broadcasts (~every {interval:.2f}s)")


def check_refill_after_depletion(port):
    """On a 1x1 map every resource target is 1; deplete food, expect refill."""
    gui = gui_connect(port)
    pl = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    pl.connect((HOST, port))
    pl.settimeout(2)
    pl.recv(64)                       # WELCOME
    pl.sendall(b"team1\n")
    time.sleep(0.2)
    pl.recv(64)                       # slots + map size
    pl.sendall(b"Take food\n")        # deplete the single tile's food

    foods = []
    deadline = time.time() + 2.0
    while time.time() < deadline:
        gui.sendall(b"bct 0 0\n")
        for ln in recvlines(gui, 0.05):
            tile = parse_bct([ln]).get((0, 0))
            if tile is not None:
                foods.append(tile[0])
    gui.close()
    pl.close()
    assert 0 in foods, f"food never depleted; saw {sorted(set(foods))}"
    zero_at = foods.index(0)
    assert 1 in foods[zero_at:], f"food never refilled after depletion; saw {foods}"
    print("  [ok] food depleted to 0 then refilled to target on 1x1 map")


def main():
    failures = []

    print("[1/2] 5x5 map, freq 10: totals + periodic broadcast")
    proc = start_server(4251, 5, 5, 10)
    try:
        check_initial_totals(4251, 5, 5)
        check_periodic_broadcast(4251, 5, 5, 10)
    except AssertionError as e:
        failures.append(str(e))
    finally:
        proc.terminate(); proc.wait()

    print("[2/2] 1x1 map, freq 100: depletion + refill")
    proc = start_server(4252, 1, 1, 100)
    try:
        check_refill_after_depletion(4252)
    except AssertionError as e:
        failures.append(str(e))
    finally:
        proc.terminate(); proc.wait()

    if failures:
        print("\nFAILED:")
        for f in failures:
            print(f"  - {f}")
        sys.exit(1)
    print("\nAll respawn checks passed.")


if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(SERVER)) if "/" in SERVER else ".")
    main()
