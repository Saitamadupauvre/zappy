#!/usr/bin/env python3
"""Test client: connects and spams Look, prints responses."""

import socket
import sys
import time

HOST = "127.0.0.1"
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 4242
TEAM = sys.argv[2] if len(sys.argv) > 2 else "team1"


def recvline(sock):
    buf = b""
    while not buf.endswith(b"\n"):
        chunk = sock.recv(1)
        if not chunk:
            raise ConnectionError("server closed connection")
        buf += chunk
    return buf.decode().strip()


def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        print(f"[connected] {HOST}:{PORT}")

        welcome = recvline(s)
        print(f"[recv] {welcome}")
        assert welcome == "WELCOME", f"unexpected: {welcome!r}"

        s.sendall(f"{TEAM}\n".encode())
        print(f"[sent] {TEAM}")

        slots = recvline(s)
        map_size = recvline(s)
        print(f"[recv] slots={slots}  map={map_size}")

        for i in range(5):
            s.sendall(b"Look\n")
            print(f"[sent] Look #{i + 1}")
            response = recvline(s)
            print(f"[recv] {response}")
            time.sleep(0.5)


if __name__ == "__main__":
    main()
