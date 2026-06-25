#!/usr/bin/env python3
"""
Test incantation command (two-phase: CHANNELING → finish).

Server to launch:
    ./zappy_server -p 4242 -x 1 -y 1 -n t1 t2 -c 5 -f 100

World 1x1:
  - All resources guaranteed ≥ 1 on tile (0,0).
  - Level 1 incantation needs 1 player + 1 linemate — always satisfied.
  - CD_INCANTATION = 300 ticks → set_client_cd(100, 300) = 3000 ms → wait 3.5s.

Tests:
  1. Single player level 1: incantation succeeds → "Elevation underway" then "Current level: 2"
  2. Same player now level 2: incantation fails (needs 2 players) → "ko"
"""

import socket
import sys
import time

HOST = "127.0.0.1"
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 4242

PASS = "\033[92mPASS\033[0m"
FAIL = "\033[91mFAIL\033[0m"

INCANTATION_CD_S = 10  # 3000 ms + 0.5s margin


def connect(team):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    s.settimeout(6.0)
    welcome = recvline(s)
    assert welcome == "WELCOME", f"bad welcome: {welcome!r}"
    s.sendall(f"{team}\n".encode())
    slots    = recvline(s)
    map_size = recvline(s)
    print(f"  connected — slots={slots} map={map_size}")
    return s


def recvline(s):
    buf = b""
    while not buf.endswith(b"\n"):
        buf += s.recv(1)
    return buf.decode().strip()


def check(label, got, expected):
    ok = got == expected
    print(f"  {label}: {PASS if ok else FAIL} (got {got!r}, want {expected!r})")
    return ok


def main():
    print("=== test_incantation ===")
    print(f"server: {HOST}:{PORT}")
    print("expected server: ./zappy_server -p 4242 -x 1 -y 1 -n t1 t2 -c 5 -f 100\n")

    results = []

    # --- Test 1: successful elevation level 1 → 2 ---
    print("[Test 1] single player level 1 incantation (expect success)")
    sa = connect("t1")

    sa.sendall(b"Incantation\n")
    print("  sent: Incantation")

    try:
        resp1 = recvline(sa)
    except socket.timeout:
        resp1 = "TIMEOUT"
    print(f"  phase1 recv: {resp1!r}")
    ok1a = check("phase1 = 'Elevation underway'", resp1, "Elevation underway")

    print(f"  waiting {INCANTATION_CD_S}s for CD to expire ...")
    time.sleep(INCANTATION_CD_S)

    try:
        resp2 = recvline(sa)
    except socket.timeout:
        resp2 = "TIMEOUT"
    print(f"  phase2 recv: {resp2!r}")
    ok1b = check("phase2 = 'Current level: 2'", resp2, "Current level: 2")

    results.append(ok1a and ok1b)

    # --- Test 2: level 2 incantation fails (need 2 players, only 1 present) ---
    print("\n[Test 2] same player now level 2, incantation should fail (ko)")
    sa.sendall(b"Incantation\n")
    print("  sent: Incantation")

    try:
        resp3 = recvline(sa)
    except socket.timeout:
        resp3 = "TIMEOUT"
    print(f"  recv: {resp3!r}")
    ok2 = check("got 'ko'", resp3, "ko")
    results.append(ok2)

    sa.close()

    print()
    if all(results):
        print(f"{PASS} all checks passed")
        sys.exit(0)
    else:
        print(f"{FAIL} some checks failed")
        sys.exit(1)


if __name__ == "__main__":
    main()
