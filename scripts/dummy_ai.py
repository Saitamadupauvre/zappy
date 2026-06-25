#!/usr/bin/env python3
"""Dummy Zappy AI — connects to server, moves randomly and broadcasts brainrot."""

import socket
import time
import random
import sys

HOST = "127.0.0.1"
PORT = 4242
TEAM = "team1"

MOVES = ["Forward\n", "Forward\n", "Forward\n", "Right\n", "Left\n"]

BRAINROT = [
    "bro_really_said_that💀",
    "nocap_fr_fr",
    "its_giving_main_character_energy",
    "slay_bestie",
    "ohio_moment",
    "W_rizz_detected",
    "this_is_so_skibidi",
    "not_the_sigma_grindset",
    "lowkey_bussin_ngl",
    "hes_so_cooked_rn",
    "the_aura_is_off_the_charts",
    "touch_grass_challenge_failed",
    "certified_hood_classic",
    "yapping_arc_incoming",
    "glazing_hard_rn🙏",
    "bro_fell_off_diff",
    "ratio+L+nocap",
    "fanum_tax_dodged",
    "delulu_behavior_detected",
    "real_and_true_bestie",
    "on_god_no_printer",
    "giving_me_the_ick_ngl",
    "understood_the_assignment_fr",
    "this_aint_it_chief",
    "big_yikes_energy",
    "sussy_behavior_detected",
    "mid_arc_tbh",
    "npc_behavior_activated",
    "the_lore_is_getting_deeper",
    "chat_is_this_real??",
]


def connect(host: str, port: int, team: str) -> socket.socket:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    # WELCOME
    data = b""
    while b"\n" not in data:
        data += s.recv(1024)

    # send team name
    s.sendall((team + "\n").encode())

    # receive slots + map size (2 lines)
    lines = 0
    data = b""
    while lines < 2:
        chunk = s.recv(1024)
        if not chunk:
            raise ConnectionError("server closed")
        data += chunk
        lines = data.count(b"\n")

    return s


def drain(s: socket.socket) -> None:
    s.setblocking(False)
    try:
        while s.recv(4096):
            pass
    except BlockingIOError:
        pass
    s.setblocking(True)


def run(host: str, port: int, team: str, delay: float) -> None:
    print(f"[AI] connecting to {host}:{port} as {team}")
    try:
        s = connect(host, port, team)
    except Exception as e:
        print(f"[AI] connection failed: {e}", file=sys.stderr)
        return

    print("[AI] connected — starting random walk")
    s.settimeout(2.0)

    move_count = 0
    broadcast_every = random.randint(3, 7)

    while True:
        if move_count > 0 and move_count % broadcast_every == 0:
            msg = random.choice(BRAINROT)
            cmd = f"Broadcast {msg}\n"
            broadcast_every = random.randint(3, 7)
        else:
            cmd = random.choice(MOVES)
        move_count += 1

        try:
            s.sendall(cmd.encode())
            # read response (ok / ko / dead)
            resp = b""
            while b"\n" not in resp:
                chunk = s.recv(256)
                if not chunk:
                    print("[AI] server closed connection")
                    return
                resp += chunk
            resp_str = resp.split(b"\n")[0].decode(errors="replace")
            if resp_str == "dead":
                print("[AI] player died")
                return
        except socket.timeout:
            drain(s)
        except Exception as e:
            print(f"[AI] error: {e}", file=sys.stderr)
            return
        time.sleep(delay)


if __name__ == "__main__":
    import argparse

    p = argparse.ArgumentParser(description="Dummy Zappy AI")
    p.add_argument("-p", "--port",  type=int,   default=4242)
    p.add_argument("-h2", "--host", type=str,   default="127.0.0.1", dest="host")
    p.add_argument("-n", "--team",  type=str,   default="team1")
    p.add_argument("-d", "--delay", type=float, default=0.3,
                   help="seconds between moves")
    args = p.parse_args()

    run(args.host, args.port, args.team, args.delay)
