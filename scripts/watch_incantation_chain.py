#!/usr/bin/env python3
"""Run the L1->L6 incantation chain with the real GUI open so you can watch.

Starts zappy_server, launches gui/zappy_gui (visible on the X display) bound to
it, then drives the same chain logic from test_incantation_chain.py at a slower
frequency with pauses between levels. Leaves the GUI up at the end so you can
look at the 4 level-6 players before it tears down.
"""
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import test_incantation_chain as T  # noqa: E402

# Slow it down so each ritual (now ~1s) and each elevation is watchable.
T.PORT = 4794
T.FREQ = 300
T.REFILL_MS = 20_000 / T.FREQ / 1000.0
T.WATCH_PAUSE = 1.2
LINGER_END = 12.0  # seconds to keep GUI open after reaching level 6


def main():
    display = os.environ.get("DISPLAY", ":1")
    srv = T.start_server()
    env = dict(os.environ, DISPLAY=display)
    gui = subprocess.Popen(
        ["./zappy_gui", "-p", str(T.PORT)],
        cwd=os.path.join(ROOT, "gui"), env=env,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"GUI launched on DISPLAY={display} (port {T.PORT}). "
          "Watch the window — 4 players elevate L1 -> L6.")
    time.sleep(2.5)  # let the window open + connect + draw the map
    if gui.poll() is not None:
        srv.terminate()
        sys.exit("GUI failed to launch (no display?)")

    try:
        T.run_chain()
        print(f"\nReached level 6. Holding GUI open {LINGER_END:.0f}s...")
        time.sleep(LINGER_END)
    finally:
        gui.terminate()
        srv.terminate()
        for p in (gui, srv):
            try:
                p.wait(2)
            except subprocess.TimeoutExpired:
                p.kill()

    print(f"\n{sum(T.results)}/{len(T.results)} checks passed -> "
          f"{'REACHED LEVEL 6' if all(T.results) else 'CHAIN FAILED'}")
    sys.exit(0 if all(T.results) else 1)


if __name__ == "__main__":
    main()
