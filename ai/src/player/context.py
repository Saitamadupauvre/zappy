from __future__ import annotations
from logger.logger import Logger
from dataclasses import dataclass, field
from collections import deque
from typing import TYPE_CHECKING
from uuid import uuid4
import datetime
import os

if TYPE_CHECKING:
    from player.player import Player
    from network.zappyClient import ZappyClient

INVENTORY_CHECK_INTERVAL = 15
FOLLOWERS_NEEDED = 5

@dataclass
class FollowerInfo:
    level: int = 1
    inventory: dict[str, int] = field(default_factory=lambda: {
        "food": 0, "linemate": 0, "deraumere": 0,
        "sibur": 0, "mendiane": 0, "phiras": 0, "thystame": 0,
    })
    last_seen_tick: int = 0

@dataclass
class BotContext:
    player: "Player"
    client: "ZappyClient"
    map_dim: tuple[int, int]
    team_name: str

    uuid: str = field(default_factory=lambda: uuid4().hex[:8])
    key: bytes | None = None
    host: str = ""
    port: int = 0

    log_filepath: str = field(init=False)

    known_resources: dict[str, tuple[int, int]] = field(default_factory=dict)
    action_queue: deque[str] = field(default_factory=deque)
    debug_enabled: bool = field(default_factory=lambda: os.getenv("ZAPPY_AI_DEBUG", "0") != "0")

    is_queen: bool = False
    role_decided: bool = False
    ping_tick: int = 0

    queen_followers: dict[str, FollowerInfo] = field(default_factory=dict)
    queen_joined_followers: set[str] = field(default_factory=set)

    queen_tick: int = 0
    queen_poll_interval: int = 20
    queen_last_poll: int = 0
    queen_last_fork: int = -30
    queen_is_forking: bool = False
    queen_pending_fork_launches: int = 0
    queen_spawned_followers: int = 0

    queen_max_followers: int = FOLLOWERS_NEEDED

    elevation_target: tuple[int, int] | None = None
    elevation_start_level: int = 1
    elevation_direction: int | None = None
    elevation_direction_fresh: bool = False
    elevation_intents: int = 0
    elevation_waiting: int = 0
    elevation_joined: bool = False

    elevation_wait_ticks: int = 0
    inventory_tick_counter: int = 0
    tick_counter: int = 0
    debug_last_position: tuple[int, int] | None = None
    debug_stationary_ticks: int = 0
    debug_last_inventory: dict[str, int] = field(default_factory=dict)
    debug_last_messages: dict[str, int] = field(default_factory=dict)

    def __post_init__(self) -> None:
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        log_dir = os.path.join("ai", "logs", f"run_{timestamp}")
        os.makedirs(log_dir, exist_ok=True)

        filename = f"logs_{self.team_name}_{os.getpid()}.csv"
        filepath = os.path.join(log_dir, filename)
        self.logger = Logger(filepath)

    def register_follower_alive(self, uuid: str, level: int, inventory: dict[str, int]) -> None:
        self.queen_followers[uuid] = FollowerInfo(
            level=level,
            inventory=dict(inventory),
            last_seen_tick=self.queen_tick,
        )

    def alive_followers(self) -> list[FollowerInfo]:
        cutoff = self.queen_tick - self.queen_poll_interval * 6
        return [f for f in self.queen_followers.values() if f.last_seen_tick >= cutoff]

    def alive_follower_count(self) -> int:
        return len(self.alive_followers())

    def followers_at_level(self, level: int) -> int:
        return sum(1 for f in self.alive_followers() if f.level == level)

    def global_inventory(self) -> dict[str, int]:
        totals: dict[str, int] = dict(self.player.inventory)
        for f in self.alive_followers():
            for stone, qty in f.inventory.items():
                totals[stone] = totals.get(stone, 0) + qty
        return totals

    def prune_dead_followers(self) -> None:
        cutoff = self.queen_tick - self.queen_poll_interval * 6
        dead = [uid for uid, f in self.queen_followers.items()
                if f.last_seen_tick < cutoff]
        for uid in dead:
            del self.queen_followers[uid]
            self.queen_spawned_followers = max(0, self.queen_spawned_followers - 1)

    def debug(self, message: str) -> None:
        if not self.debug_enabled:
            return

        role = "Q" if self.is_queen else "F"
        food = self.player.inventory.get("food", "?")
        queue_len = len(self.action_queue)
        print(
            f"[AI:{role}:{self.uuid} t={self.tick_counter} qt={self.queen_tick} "
            f"lvl={self.player.level} food={food} pos={self.player.position} "
            f"q={queue_len}] {message}",
            flush=True,
        )

    def debug_every(self, key: str, interval: int, message: str) -> None:
        last_tick = self.debug_last_messages.get(key, -interval)
        if self.tick_counter - last_tick < interval:
            return

        self.debug_last_messages[key] = self.tick_counter
        self.debug(message)
