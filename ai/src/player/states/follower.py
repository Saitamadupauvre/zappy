from __future__ import annotations
import random
from player.fsm import State
from player.context import BotContext
from player.commands import Commands
from network.broadcast import (
    MsgType, encode,
    encode_alive, encode_join,
)
from player.queen.state import FOOD_LOW, FOOD_CRITICAL, FOOD_HIGH

_F = Commands.FORWARD.build()
_R = Commands.RIGHT.build()
_L = Commands.LEFT.build()

_K_TO_CMDS: dict[int, list[str]] = {
    1: [_F],
    2: [_F, _L, _F],
    3: [_L, _F],
    4: [_L, _F, _L, _F],
    5: [_R, _R, _F],
    6: [_R, _F, _R, _F],
    7: [_R, _F],
    8: [_F, _R, _F],
}

STALE_DIRECTION_TIMEOUT = 300
JOINED_REFRESH_TIMEOUT = 300


class _FS_Survive(State):
    def enter(self, ctx: BotContext) -> None:
        ctx.action_queue.clear()
        ctx.debug("enter _FS_Survive")

    def update(self, ctx: BotContext) -> State | None:
        food = ctx.player.inventory.get("food", 0)
        if food >= FOOD_HIGH:
            return _FS_Collect()

        if ctx.elevation_target is not None:
            return _FS_Gather()

        if not ctx.action_queue:
            _enqueue_food(ctx)
        return None

    def exit(self, ctx: BotContext) -> None:
        pass


class _FS_Gather(State):
    def enter(self, ctx: BotContext) -> None:
        ctx.action_queue.clear()
        ctx.debug("enter _FS_Gather")

    def update(self, ctx: BotContext) -> State | None:
        food = ctx.player.inventory.get("food", 0)
        if food < FOOD_LOW:
            ctx.debug(f"incant target ignored for food ({food})")
            ctx.elevation_target = None
            ctx.elevation_direction_fresh = False
            ctx.elevation_wait_ticks = 0
            return _FS_Survive()

        if ctx.elevation_joined:
            return _FS_Joined()

        if ctx.elevation_target is None:
            return _FS_Collect()

        _handle_go_to_queen(ctx)
        return None

    def exit(self, ctx: BotContext) -> None:
        pass


class _FS_Joined(State):
    def enter(self, ctx: BotContext) -> None:
        ctx.debug("enter _FS_Joined")

    def update(self, ctx: BotContext) -> State | None:
        food = ctx.player.inventory.get("food", 0)
        if food < FOOD_LOW:
            ctx.debug(f"joined but food low ({food}) -> leave incant and eat")
            ctx.action_queue.clear()
            ctx.elevation_target = None
            ctx.elevation_joined = False
            ctx.elevation_direction_fresh = False
            ctx.elevation_wait_ticks = 0
            return _FS_Survive()

        if ctx.player.level > ctx.elevation_start_level:
            ctx.debug(f"joined level changed to {ctx.player.level} -> resume")
            ctx.elevation_joined = False
            ctx.elevation_target = None
            ctx.elevation_wait_ticks = 0
            ctx.action_queue.clear()
            return _FS_Collect()

        if ctx.elevation_target is None:
            ctx.elevation_joined = False
            ctx.elevation_wait_ticks = 0
            ctx.action_queue.clear()
            return _FS_Collect()

        if getattr(ctx, "elevation_underway", False):
            ctx.elevation_wait_ticks = 0
            return None

        ctx.elevation_wait_ticks += 1
        if ctx.elevation_wait_ticks >= JOINED_REFRESH_TIMEOUT:
            ctx.debug(
                f"leave joined incant: no refresh for "
                f"{ctx.elevation_wait_ticks} ticks"
            )
            ctx.elevation_target = None
            ctx.elevation_joined = False
            ctx.elevation_direction = None
            ctx.elevation_direction_fresh = False
            ctx.elevation_wait_ticks = 0
            return _FS_Collect()

        if food < FOOD_HIGH and not ctx.action_queue:
            _enqueue_joined_food(ctx)
        return None

    def exit(self, ctx: BotContext) -> None:
        pass


class _FS_Collect(State):
    def enter(self, ctx: BotContext) -> None:
        ctx.debug("enter _FS_Collect")

    def update(self, ctx: BotContext) -> State | None:
        food = ctx.player.inventory.get("food", 0)
        if food < FOOD_LOW:
            return _FS_Survive()

        if ctx.elevation_target is not None:
            return _FS_Gather()

        if food < FOOD_HIGH and not ctx.action_queue:
            ctx.debug_every("below_high_food", 20, f"food below high ({food}) -> collect food")
            _enqueue_food(ctx)
            return None

        if not ctx.action_queue:
            ctx.debug_every("collect_resources", 25, "collect resources")
            _enqueue_collect(ctx)

        return None

    def exit(self, ctx: BotContext) -> None:
        pass


class FollowerMainState(State):
    _sub: State

    def enter(self, ctx: BotContext) -> None:
        ctx.action_queue.clear()
        ctx.elevation_target  = None
        ctx.elevation_joined  = False
        ctx.elevation_direction_fresh = False
        ctx.elevation_wait_ticks = 0
        ctx.debug("enter FollowerMainState")
        _send_alive(ctx)
        self._sub = _FS_Collect()
        self._sub.enter(ctx)

    def update(self, ctx: BotContext) -> State | None:
        next_sub = self._sub.update(ctx)
        if next_sub is not None:
            self._sub.exit(ctx)
            self._sub = next_sub
            self._sub.enter(ctx)
        return None

    def exit(self, ctx: BotContext) -> None:
        self._sub.exit(ctx)


def _handle_go_to_queen(ctx: BotContext) -> None:
    if ctx.action_queue:
        return

    if not ctx.elevation_direction_fresh:
        ctx.elevation_wait_ticks += 1
        if ctx.elevation_wait_ticks >= STALE_DIRECTION_TIMEOUT:
            ctx.debug(
                f"abandon incant target: no fresh direction for "
                f"{ctx.elevation_wait_ticks} ticks"
            )
            ctx.elevation_target = None
            ctx.elevation_direction = None
            ctx.elevation_direction_fresh = False
            ctx.elevation_wait_ticks = 0
            _enqueue_food(ctx)
            return

        ctx.debug_every(
            "wait_fresh_direction",
            10,
            f"blocked going queen: waiting fresh direction target={ctx.elevation_target}",
        )
        return

    k = ctx.elevation_direction
    ctx.elevation_direction_fresh = False
    ctx.elevation_wait_ticks = 0
    ctx.debug(f"go queen direction={k}")

    if k == 0:
        _arrive_on_tile(ctx)
        return

    ctx.action_queue.extend(_K_TO_CMDS.get(k, [_F]))
    ctx.action_queue.append(Commands.LOOK.build())


def _arrive_on_tile(ctx: BotContext) -> None:
    ctx.debug("arrived on queen tile -> drop stones and JOIN")
    _drop_stones(ctx)
    ctx.action_queue.appendleft(Commands.BROADCAST.build(
        encode(ctx.team_name, MsgType.JOIN,
               encode_join(ctx.uuid, ctx.player.level))
    ))
    ctx.elevation_joined = True
    ctx.elevation_wait_ticks = 0
    ctx.debug("joined incantation")


def _drop_stones(ctx: BotContext) -> None:
    next_level   = ctx.player.level + 1
    requirements = ctx.player.ELEVATION_REQUIREMENTS.get(next_level, {})
    for stone, needed in requirements.items():
        have = ctx.player.inventory.get(stone, 0)
        for _ in range(min(have, needed)):
            resp = ctx.client.send_action(Commands.SET_OBJECT.build(stone))
            if resp == "ok":
                ctx.player.inventory[stone] -= 1
                ctx.debug(f"dropped {stone} remaining={ctx.player.inventory[stone]}")


def _enqueue_collect(ctx: BotContext) -> None:
    from player.vision import Vision
    from player.navigation import navigate

    vision = ctx.player.vision
    if vision is None:
        ctx.debug("collect: no vision -> look")
        ctx.action_queue.append(Commands.LOOK.build())
        return

    food = ctx.player.inventory.get("food", 0)
    if food < FOOD_HIGH:
        idx = vision.find_nearest("food")
        if idx is not None:
            target = Vision.get_xy_tile(idx, ctx.player, ctx.map_dim)
            if target != ctx.player.position:
                path = navigate(ctx.player.position, ctx.player.direction, target, ctx.map_dim)
                ctx.debug(f"collect food target={target} path={path}")
                ctx.action_queue.extend(path)
            ctx.action_queue.append(Commands.TAKE_OBJECT.build("food"))
            ctx.action_queue.append(Commands.LOOK.build())
            return

    needed = ctx.player.nearest_missing_stone()
    if needed:
        idx = vision.find_nearest(needed)
        if idx is not None:
            target = Vision.get_xy_tile(idx, ctx.player, ctx.map_dim)
            if target != ctx.player.position:
                path = navigate(ctx.player.position, ctx.player.direction, target, ctx.map_dim)
                ctx.debug(f"collect stone={needed} target={target} path={path}")
                ctx.action_queue.extend(path)
            ctx.action_queue.append(Commands.TAKE_OBJECT.build(needed))
            ctx.action_queue.append(Commands.LOOK.build())
            return

    _enqueue_wander(ctx)


def _enqueue_food(ctx: BotContext) -> None:
    from player.vision import Vision
    from player.navigation import navigate

    vision = ctx.player.vision
    if vision is None:
        ctx.debug("food: no vision -> look")
        ctx.action_queue.append(Commands.LOOK.build())
        return

    tile_index = vision.find_nearest("food")
    if tile_index is not None:
        target = Vision.get_xy_tile(tile_index, ctx.player, ctx.map_dim)
    else:
        target = ctx.known_resources.get("food")
        if target is not None and target == ctx.player.position:
            if "food" in ctx.known_resources:
                del ctx.known_resources["food"]
            target = None

    if target is not None:
        if target != ctx.player.position:
            path = navigate(ctx.player.position, ctx.player.direction, target, ctx.map_dim)
            ctx.debug(f"food target={target} path={path}")
            ctx.action_queue.extend(path)
        ctx.action_queue.append(Commands.TAKE_OBJECT.build("food"))
        ctx.action_queue.append(Commands.LOOK.build())
        return

    turn = random.choice([Commands.LEFT.build(), Commands.RIGHT.build(), ""])
    ctx.debug(f"food wander turn={turn or 'none'}")
    if turn:
        ctx.action_queue.append(turn)
    ctx.action_queue.append(Commands.FORWARD.build())
    ctx.action_queue.append(Commands.LOOK.build())


def _enqueue_joined_food(ctx: BotContext):
    from player.vision import Vision

    vision = ctx.player.vision
    if vision is None:
        ctx.debug("joined food: no vision -> look")
        ctx.action_queue.append(Commands.LOOK.build())
        return

    if vision.tiles[0].food > 0:
        ctx.debug("joined food: take food on tile")
        ctx.action_queue.append(Commands.TAKE_OBJECT.build("food"))
    else:
        ctx.debug_every("joined_no_food", 10, "joined food: no food on tile -> look")
        ctx.action_queue.append(Commands.LOOK.build())


def _enqueue_wander(ctx: BotContext) -> None:
    turn = random.choice([Commands.LEFT.build(), Commands.RIGHT.build(), ""])
    ctx.debug(f"wander turn={turn or 'none'}")
    if turn:
        ctx.action_queue.append(turn)
    ctx.action_queue.append(Commands.FORWARD.build())
    ctx.action_queue.append(Commands.LOOK.build())


def _send_alive(ctx: BotContext) -> None:
    ctx.debug(f"send ALIVE inventory={ctx.player.inventory}")
    ctx.client.send_action(Commands.BROADCAST.build(
        encode(ctx.team_name, MsgType.ALIVE,
               encode_alive(ctx.uuid, ctx.player.level, ctx.player.inventory))
    ))
