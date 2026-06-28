from __future__ import annotations
from player.navigation import navigate
from player.commands import Commands
from player.process import Process
from player.context import BotContext, FOLLOWERS_NEEDED
from player.vision import Vision
from player.fsm import State
from network.broadcast import (
    MsgType, encode,
    encode_pong, encode_poll,
    encode_incant, encode_done,
)
import random

FOOD_CRITICAL = 5  # hard floor - stop everything
FOOD_LOW      = 10 # prefer to eat rather than collect stones
FOOD_INCANT   = 30 # minimum food to accept an incantation rally
FOOD_HIGH     = 35 # comfortable, can focus on stones / incantations

POLL_INTERVAL    = 20
REBROADCAST_INT  = 8
FORK_COOLDOWN    = 20
FOLLOWER_JOIN_TIMEOUT = 600
RALLY_HOLD_PLAYERS = 2

class _QS_Survive(State):
    def enter(self, ctx: BotContext) -> None:
        ctx.action_queue.clear()

    def update(self, ctx: BotContext) -> State | None:
        if ctx.player.inventory.get("food", 0) >= FOOD_HIGH:
            return _QS_Idle()
        if not ctx.action_queue:
            self._enqueue_food(ctx)
        return None

    def exit(self, ctx: BotContext) -> None:
        pass

    def _enqueue_food(self, ctx: BotContext) -> None:
        vision = ctx.player.vision
        if vision is None:
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
                ctx.action_queue.extend(path)
            ctx.action_queue.append(Commands.TAKE_OBJECT.build("food"))
            ctx.action_queue.append(Commands.LOOK.build())
            return

        turn = random.choice([Commands.LEFT.build(), Commands.RIGHT.build(), ""])
        if turn:
            ctx.action_queue.append(turn)
        ctx.action_queue.append(Commands.FORWARD.build())
        ctx.action_queue.append(Commands.LOOK.build())


class _QS_Idle(State):

    def enter(self, ctx: BotContext) -> None:
        ctx.action_queue.clear()
        for f in ctx.queen_followers.values():
            f.last_seen_tick = ctx.queen_tick

    def update(self, ctx: BotContext) -> State | None:
        food = ctx.player.inventory.get("food", 0)

        if food < FOOD_LOW:
            ctx.elevation_intents = 0
            return _QS_Survive()

        if ctx.queen_tick - ctx.queen_last_poll >= POLL_INTERVAL:
            ctx.queen_last_poll = ctx.queen_tick
            ctx.prune_dead_followers()
            ctx.action_queue.append(Commands.BROADCAST.build(
                encode(ctx.team_name, MsgType.POLL, encode_poll())
            ))

        alive = ctx.alive_follower_count()
        expected = max(alive, ctx.queen_spawned_followers + ctx.queen_pending_fork_launches)
        if expected < ctx.queen_max_followers and not ctx.queen_is_forking:
            if Commands.CONNECT_NBR.build() not in ctx.action_queue:
                ctx.action_queue.append(Commands.CONNECT_NBR.build())

        self._maybe_spawn(ctx)

        if self._can_incant(ctx):
            return _QS_GatherFollowers()

        if not ctx.action_queue:
            if food < FOOD_HIGH:
                self._enqueue_food(ctx)
            else:
                self._enqueue_wander(ctx)

        return None

    def exit(self, ctx: BotContext) -> None:
        pass

    def _enqueue_food(self, ctx: BotContext) -> None:
        vision = ctx.player.vision
        if vision is None:
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
                ctx.action_queue.extend(path)
            ctx.action_queue.append(Commands.TAKE_OBJECT.build("food"))
            ctx.action_queue.append(Commands.LOOK.build())
            return

        turn = random.choice([Commands.LEFT.build(), Commands.RIGHT.build(), ""])
        if turn:
            ctx.action_queue.append(turn)
        ctx.action_queue.append(Commands.FORWARD.build())
        ctx.action_queue.append(Commands.LOOK.build())

    def _maybe_spawn(self, ctx: BotContext) -> None:
        alive = ctx.alive_follower_count()
        connecting = max(0, ctx.queen_spawned_followers - alive)
        expected = max(alive, ctx.queen_spawned_followers + ctx.queen_pending_fork_launches)
        if expected >= ctx.queen_max_followers:
            return

        if ctx.queen_is_forking:
            return

        slots = getattr(ctx.player, "available_slot", -1)

        unclaimed_slots = max(0, slots - connecting)
        if unclaimed_slots > 0:
            needed = ctx.queen_max_followers - expected
            to_connect = min(unclaimed_slots, needed)
            for _ in range(to_connect):
                self._do_connect(ctx)
            ctx.player.available_slot = 0

        elif ctx.queen_tick - ctx.queen_last_fork >= FORK_COOLDOWN:
            ctx.queen_is_forking = True
            ctx.action_queue.append(Commands.FORK.build())


    def _do_connect(self, ctx: BotContext) -> None:
        from player.zappyAI import ZappyAI
        Process(
            name=f"AI_{ctx.team_name}_Connected",
            function=ZappyAI.launch_bot,
            args=[ctx.team_name, ctx.host, ctx.port, False, ctx.key, ctx.debug_enabled],
        ).start()
        ctx.queen_spawned_followers += 1


    def _can_incant(self, ctx: BotContext) -> bool:
        level = ctx.player.level
        if level >= 8:
            return False

        if ctx.player.inventory.get("food", 0) < FOOD_HIGH:
            return False

        at_level = ctx.followers_at_level(level)
        if at_level < ctx.queen_max_followers:
            return False
        
        for uuid, info in ctx.queen_followers.items():
            if info.level == level:
                follower_food = info.inventory.get("food", 0)
                
                if follower_food < FOOD_INCANT:
                    ctx.debug(f"Not enough food for {uuid} (current food : {follower_food})")
                    return False

        requirements = ctx.player.ELEVATION_REQUIREMENTS.get(level + 1, {})
        global_inv = ctx.global_inventory()
        for stone, qty in requirements.items():
            if global_inv.get(stone, 0) < qty:
                return False

        return True

    def _enqueue_wander(self, ctx: BotContext) -> None:
        vision = ctx.player.vision
        if vision is None:
            ctx.action_queue.append(Commands.LOOK.build())
            return

        needed = ctx.player.nearest_missing_stone()
        if needed:
            idx = vision.find_nearest(needed)
            if idx is not None:
                target = Vision.get_xy_tile(idx, ctx.player, ctx.map_dim)
                if target != ctx.player.position:
                    path = navigate(ctx.player.position, ctx.player.direction, target, ctx.map_dim)
                    ctx.action_queue.extend(path)
                    ctx.action_queue.append(Commands.TAKE_OBJECT.build(needed))
                    ctx.action_queue.append(Commands.LOOK.build())
                    return

        turn = random.choice([Commands.LEFT.build(), Commands.RIGHT.build(), ""])
        if turn:
            ctx.action_queue.append(turn)
        ctx.action_queue.append(Commands.FORWARD.build())
        ctx.action_queue.append(Commands.LOOK.build())


class _QS_GatherFollowers(State):

    def enter(self, ctx: BotContext) -> None:
        self._wait_ticks = 0
        ctx.elevation_intents = 0
        ctx.queen_joined_followers.clear()
        w, h = ctx.map_dim
        self._max_wait = max(FOLLOWER_JOIN_TIMEOUT, (w + h) * 2)
        ctx.action_queue.clear()
        self._broadcast_incant(ctx)

    def update(self, ctx: BotContext) -> State | None:
        food = ctx.player.inventory.get("food", 0)
        players_on_tile = self._players_on_tile(ctx)

        if food < FOOD_LOW:
            ctx.elevation_intents = 0
            ctx.queen_joined_followers.clear()
            ctx.debug(f"gather food too low ({food}) -> abort and survive")
            return _QS_Survive()

        self._wait_ticks += 1

        if food > FOOD_HIGH + 5 and self._wait_ticks % 8 == 0 and players_on_tile > 1:
            ctx.action_queue.append(Commands.SET_OBJECT.build("food"))
            ctx.player.inventory["food"] -= 1

        if food < FOOD_LOW and not ctx.action_queue:
            self._enqueue_food(ctx)

        if self._wait_ticks % REBROADCAST_INT == 0:
            self._broadcast_incant(ctx)

        if len(ctx.queen_joined_followers) >= FOLLOWERS_NEEDED and players_on_tile >= FOLLOWERS_NEEDED + 1:
            return _QS_Incanting()

        if self._wait_ticks >= self._max_wait:
            ctx.elevation_intents = 0
            ctx.queen_joined_followers.clear()
            return _QS_Idle()

        if not ctx.action_queue:
            self._enqueue_rally_action(ctx, players_on_tile)

        return None

    def exit(self, ctx: BotContext) -> None:
        pass

    @staticmethod
    def _players_on_tile(ctx: BotContext) -> int:
        if ctx.player.vision is None or not ctx.player.vision.tiles:
            return 1
        return ctx.player.vision.tiles[0].players

    def _enqueue_rally_action(self, ctx: BotContext, players_on_tile: int) -> None:
        # Stand still while waiting for followers; eat from current tile if hungry
        food = ctx.player.inventory.get("food", 0)
        if food < FOOD_HIGH:
            vision = ctx.player.vision
            if vision is not None and len(vision.tiles) > 0 and vision.tiles[0].food > 0:
                ctx.action_queue.append(Commands.TAKE_OBJECT.build("food"))
                return
        ctx.action_queue.append(Commands.LOOK.build())

    def _enqueue_food(self, ctx: BotContext) -> None:
        vision = ctx.player.vision
        if vision is None:
            ctx.action_queue.append(Commands.LOOK.build())
            return

        tile_index = vision.find_nearest("food")
        if tile_index is not None:
            target = Vision.get_xy_tile(tile_index, ctx.player, ctx.map_dim)
        else:
            target = ctx.known_resources.get("food")

        if target is not None:
            if target != ctx.player.position:
                path = navigate(ctx.player.position, ctx.player.direction, target, ctx.map_dim)
                ctx.action_queue.extend(path)
            ctx.action_queue.append(Commands.TAKE_OBJECT.build("food"))
            ctx.action_queue.append(Commands.LOOK.build())
            return

        turn = random.choice([Commands.LEFT.build(), Commands.RIGHT.build(), ""])
        if turn:
            ctx.action_queue.append(turn)
        ctx.action_queue.append(Commands.FORWARD.build())
        ctx.action_queue.append(Commands.LOOK.build())

    def _enqueue_wander(self, ctx: BotContext) -> None:
        vision = ctx.player.vision
        if vision is None:
            ctx.action_queue.append(Commands.LOOK.build())
            return

        needed = ctx.player.nearest_missing_stone()
        if needed:
            idx = vision.find_nearest(needed)
            if idx is not None:
                target = Vision.get_xy_tile(idx, ctx.player, ctx.map_dim)
                if target != ctx.player.position:
                    path = navigate(ctx.player.position, ctx.player.direction, target, ctx.map_dim)
                    ctx.action_queue.extend(path)
                    ctx.action_queue.append(Commands.TAKE_OBJECT.build(needed))
                    ctx.action_queue.append(Commands.LOOK.build())
                    return

        turn = random.choice([Commands.LEFT.build(), Commands.RIGHT.build(), ""])
        if turn:
            ctx.action_queue.append(turn)
        ctx.action_queue.append(Commands.FORWARD.build())
        ctx.action_queue.append(Commands.LOOK.build())

    def _broadcast_incant(self, ctx: BotContext) -> None:
        ctx.action_queue.append(Commands.BROADCAST.build(
        encode(ctx.team_name, MsgType.INCANT,
               encode_incant(ctx.player.level, FOLLOWERS_NEEDED, ctx.player.position))
        ))

class _QS_Incanting(State):

    def enter(self, ctx: BotContext) -> None:
        pass

    def update(self, ctx: BotContext) -> State | None:
        self._try_incantation(ctx)
        return _QS_Idle()

    def exit(self, ctx: BotContext) -> None:
        ctx.action_queue.appendleft(Commands.BROADCAST.build(
            encode(ctx.team_name, MsgType.DONE,
                   encode_done(ctx.player.level, ctx.player.position))
        ))
        ctx.elevation_intents = 0
        ctx.debug("exit QS_Incanting broadcast DONE")

    def _try_incantation(self, ctx: BotContext) -> None:
        next_level   = ctx.player.level + 1
        requirements = ctx.player.ELEVATION_REQUIREMENTS.get(next_level, {})

        for stone, qty in requirements.items():
            for _ in range(qty):
                ctx.action_queue.appendleft(Commands.SET_OBJECT.build(stone))

        response = ctx.client.send_action(Commands.INCANT.build())
        ctx.debug(f"incant response={response}")

        if response == "Elevation underway":
            final = ctx.client.wait_for_incantation_result()
            ctx.debug(f"incant final={final}")
            if "Current level" in final:
                ctx.player.level += 1
                for stone, qty in requirements.items():
                    ctx.player.inventory[stone] = max(
                        0, ctx.player.inventory.get(stone, 0) - qty
                    )
            else:
                self._take_back_stones(ctx, requirements)
        else:
            self._take_back_stones(ctx, requirements)

    @staticmethod
    def _take_back_stones(ctx: BotContext, requirements: dict) -> None:
        for stone, qty in requirements.items():
            for _ in range(qty):
                ctx.action_queue.appendleft(Commands.TAKE_OBJECT.build(stone))


class QueenState(State):

    def enter(self, ctx: BotContext) -> None:
        ctx.is_queen = True
        ctx.debug("become queen -> broadcast PONG")
        ctx.client.send_action(Commands.BROADCAST.build(
            encode(ctx.team_name, MsgType.PONG, encode_pong())
        ))
        self._sub: State = _QS_Idle()
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
