
from network.broadcast import *
from network.zappyClient import ZappyClient
from player.navigation import navigate
from player.enum import Direction, Status
from player.player import Player, DEPLACEMENT
from player.commands import Commands

PLAYERS_REQUIRED = {2: 1, 3: 2, 4: 2, 5: 4, 6: 4, 7: 6, 8: 6}

def compute_max_wait(map_dim: tuple[int, int], level: int) -> int:
    """
    Computes a reasonable timeout for incantation coordination,
    scaled by map size (max travel distance) and number of players required.

    - Map size factor: half the map's largest dimension (worst-case travel distance)
    - Level factor: more players required → more time to gather them
    """

    w, h = map_dim
    map_factor = max(w, h) // 2 # operator '//' = Floor Division (5 // 2 = 2)

    players_needed = PLAYERS_REQUIRED.get(level + 1, 1)
    level_factor = players_needed

    return max(10, map_factor * level_factor)

class Elevation:
    """
    Handles incantation coordination: broadcasting INCANT, collecting JOINs,
    checking tile readiness, and launching the incantation command.
    """

    _waiting_players: int
    _nav_queue: list[str]
    _wait_ticks: int
    _MAX_WAIT: int
    _CANDIDATE_WAIT: int
    is_leader: bool
    is_candidate: bool
    _candidate_ticks: int
    _best_known_pos : tuple[int, int] | None
    _target_leader_pos: tuple[int, int] | None

    def __init__(self, candidate_wait: int = 5):
        self._waiting_players = 0
        self._nav_queue = []
        self._wait_ticks = 0
        self._MAX_WAIT = 20
        self._CANDIDATE_WAIT = candidate_wait
        self.is_leader = False
        self.is_candidate = False
        self._candidate_ticks = 0
        self._best_known_pos = None
        self._target_leader_pos = None

    def update(self, player: Player, client: ZappyClient, map_dim: tuple[int, int]) -> None:
        """
        Called every tick when player.status == Status.ELEVATING.
        Drives the full elevation state machine.
        """

        if self._nav_queue:
            command = self._nav_queue.pop(0)
            self._move(command, player, client, map_dim)

            if not self._nav_queue and self._target_leader_pos and not self.is_leader and not self.is_candidate:
                self._drop_my_stones(player, client)
                client.send_action(Commands.BROADCAST.build(
                    encode(player.team_name, MsgType.JOIN, encode_join(player.level))
                ))
                self._has_joined = True

            return

        if self.is_candidate and not self.is_leader:
            self._candidate_ticks += 1
            if self._candidate_ticks >= self._CANDIDATE_WAIT:
                self.is_leader = True
                self.is_candidate = False
            return

        if not self.is_leader:
            self._wait_ticks += 1
            if self._wait_ticks >= self._MAX_WAIT:
                self._wait_ticks = 0
                player.status = Status.COLLECTING
            return

        if self._waiting_players > 0:
            self._wait_ticks += 1
            if self._wait_ticks < self._MAX_WAIT:
                return

            self._wait_ticks = 0

        self._look_and_evaluate(player, client)

    def on_incant_received(self, payload: str, player: Player, client: ZappyClient, map_dim: tuple[int, int]) -> None:
        """Called when an INCANT broadcast is received."""

        result = decode_incant(payload)
        if result is None:
            return

        level, missing, x, y = result
        if level != player.level or missing <= 0:
            return

        candidate_pos = (x, y)

        # logique de regroupement, ici si on veut faire un truc plus élaboré
        # ex: aller sur la personne la plus proche
        if self.is_candidate or self.is_leader:
            if candidate_pos < player.position:
                self.is_leader = False
                self.is_candidate = False
                self._waiting_players = 0
            else:
                return

        player.status = Status.ELEVATING
        self._target_leader_pos = candidate_pos
        self._nav_queue = navigate(player.position, player.direction, candidate_pos, map_dim)

    def on_join_received(self, payload: str, player: Player) -> None:
        """Called when a JOIN broadcast is received."""

        level = decode_join(payload)
        if level != player.level:
            return

        if self._waiting_players > 0:
            self._waiting_players -= 1

    def on_done_received(self, payload: str, player: Player) -> None:
        """Called when a DONE broadcast is received — a follower can resume."""

        if self.is_leader:
            return

        result = decode_done(payload)
        if result is None:
            return

        level, x, y = result
        if self._target_leader_pos != (x, y):
            return

        player.level = level
        player.status = Status.SURVIVING
        self._target_leader_pos = None

    def become_candidate(self, player, client, map_dim: tuple[int, int]) -> None:
        """A bot with enough stones announces itself as a candidate leader."""

        self.is_candidate = True
        self._candidate_ticks = 0
        self._best_known_pos = player.position
        self._MAX_WAIT = compute_max_wait(map_dim, player.level)
        self.broadcast_incant(player, client)

    def broadcast_incant(self, player: Player, client: ZappyClient) -> None:
        """Broadcasts INCANT with current position and missing player count."""

        players_on_tile = player.vision.tiles[0].players if player.vision else 1
        needed = PLAYERS_REQUIRED.get(player.level + 1, 1)
        missing = needed - players_on_tile

        if missing > 0:
            payload = encode_incant(player.level, missing, player.position)
            client.send_action(Commands.BROADCAST.build(encode(player.team_name, MsgType.INCANT, payload)))
            self._waiting_players = missing

    def _move(self, command: str, player: Player, client: ZappyClient, map_dim: tuple[int, int]) -> None:
        response = client.send_action(command)
        if response == "ok":
            if command == "Forward":
                player.position = DEPLACEMENT[player.direction](*player.position, *map_dim)
            elif command == "Right":
                player.direction = Direction((player.direction.value + 1) % 4)
            elif command == "Left":
                player.direction = Direction((player.direction.value - 1) % 4)

    def _drop_my_stones(self, player: Player, client: ZappyClient) -> None:
        """Drop every stone I'm carrying that's needed for the next level."""

        next_level = player.level + 1
        requirements = player.ELEVATION_REQUIREMENTS.get(next_level, {})
        for stone, needed in requirements.items():
            if needed == 0:
                continue
            have = player.inventory.get(stone, 0)
            for _ in range(have):
                response = client.send_action(Commands.SET_OBJECT.build(stone))
                if response == "ok":
                    player.inventory[stone] -= 1

    def _look_and_evaluate(self, player: Player, client: ZappyClient) -> None:
        """After navigation, check how many players are on the tile."""

        if not self.is_leader:
            return

        look_response = client.send_action(Commands.LOOK.build())
        player.update_vision(look_response)

        players_on_tile = player.vision.tiles[0].players if player.vision else 0
        needed_players = PLAYERS_REQUIRED.get(player.level + 1, 1)

        if players_on_tile >= needed_players:
            self._waiting_players = 0
            self._try_incantation(player, client)
        else:
            self.broadcast_incant(player, client)

    def _try_incantation(self, player: Player, client: ZappyClient) -> None:
        next_level = player.level + 1
        requirements = player.ELEVATION_REQUIREMENTS.get(next_level, {})

        for stone, qty in requirements.items():
            for _ in range(qty):
                client.send_action(Commands.SET_OBJECT.build(stone))

        response = client.send_action(Commands.INCANT.build())
        
        if response == "Elevation underway":
            final_response = client.wait_for_incantation_result()
            
            if "Current level" in final_response:
                player.level += 1
                for stone, qty in requirements.items():
                    player.inventory[stone] = max(0, player.inventory.get(stone, 0) - qty)
            else:
                for stone, qty in requirements.items():
                    for _ in range(qty):
                        client.send_action(Commands.TAKE_OBJECT.build(stone))
        else:
            for stone, qty in requirements.items():
                for _ in range(qty):
                    client.send_action(Commands.TAKE_OBJECT.build(stone))

        client.send_action(Commands.BROADCAST.build(
            encode(player.team_name, MsgType.DONE, encode_done(player.level, player.position))
        ))

        player.status = Status.SURVIVING
        self.is_leader = False
