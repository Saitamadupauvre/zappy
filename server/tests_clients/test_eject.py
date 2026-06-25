#!/usr/bin/env python3
"""
Test eject command.

Server to launch:
    ./zappy_server -p 4242 -x 1 -y 1 -n t1 t2 -c 5 -f 100

World 1x1 forces both players onto the same tile (wrapping).
Player A ejects -> player B receives "eject: K", A receives "ok".
"""

import socket
import sys
import time
import threading

HOST  = "127.0.0.1"
PORT  = int(sys.argv[1]) if len(sys.argv) > 1 else 4242

PASS = "\033[92mPASS\033[0m"
FAIL = "\033[91mFAIL\033[0m"


def connect(team):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    s.settimeout(3.0)

    welcome = recvline(s)
    assert welcome == "WELCOME", f"bad welcome: {welcome!r}"
    s.sendall(f"{team}\n".encode())
    slots    = recvline(s)
    map_size = recvline(s)
    print(f"  [{team}] connected — slots={slots} map={map_size}")
    return s


def recvline(s):
    buf = b""
    while not buf.endswith(b"\n"):
        buf += s.recv(1)
    return buf.decode().strip()


def main():
    print("=== test_eject ===")
    print(f"server: {HOST}:{PORT}")
    print("expected server: ./zappy_server -p 4242 -x 1 -y 1 -n t1 t2 -c 5 -f 100\n")

    sa = connect("t1")
    sb = connect("t1")

    # collect B's eject notification in background
    b_got = []
    def recv_b():
        try:
            b_got.append(recvline(sb))
        except Exception as e:
            b_got.append(f"ERROR: {e}")

    t = threading.Thread(target=recv_b)
    t.start()

    time.sleep(0.1)

    sa.sendall(b"Eject\n")
    print("  [A] sent: Eject")

    try:
        a_resp = recvline(sa)
    except socket.timeout:
        a_resp = "TIMEOUT"
    print(f"  [A] recv: {a_resp!r}")

    t.join(timeout=3.0)
    b_resp = b_got[0] if b_got else "TIMEOUT"
    print(f"  [B] recv: {b_resp!r}")

    ok_a = a_resp == "ok"
    ok_b = b_resp.startswith("eject:")
    print(f"\n  A got 'ok'        : {PASS if ok_a else FAIL} (got {a_resp!r})")
    print(f"  B got 'eject: K'  : {PASS if ok_b else FAIL} (got {b_resp!r})")

    # verify A can act again after cd (7/100 = 70ms)
    time.sleep(0.15)
    sa.sendall(b"Look\n")
    look = recvline(sa)
    ok_cd = look.startswith("[")
    print(f"  A cd cleared (Look): {PASS if ok_cd else FAIL} (got {look!r})")

    sa.close()
    sb.close()

    if ok_a and ok_b and ok_cd:
        print(f"\n{PASS} all checks passed")
        sys.exit(0)
    else:
        print(f"\n{FAIL} some checks failed")
        sys.exit(1)


if __name__ == "__main__":
    main()
