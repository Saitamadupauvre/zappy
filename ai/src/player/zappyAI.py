from __future__ import annotations
import signal
import os

from player.states.discovery import DiscoveryState
from network.zappyClient import ZappyClient
from player.queen.state import FOOD_LOW, FOOD_HIGH, FOOD_INCANT
from network.broadcast import set_encryption_key
from player.commands import Commands
from player.context import BotContext, INVENTORY_CHECK_INTERVAL
from player.player import Player
from player.enum import Movement
from player.fsm import FSM
from player.process import Process
from player.enum import Movement

from network.broadcast import (
    Message, MsgType, decode,
    decode_incant, decode_join, decode_done, decode_resource,
    decode_alive, encode, encode_pong, encode_alive,
)

def _reap_children(signum, frame):
    try:
        while True:
            pid, _ = os.waitpid(-1, os.WNOHANG)
            if pid == 0:
                break
    except ChildProcessError:
        pass

class ZappyAI:
    def __init__(self, team_name: str, host: str, port: int, is_genesis: bool, key: bytes | None = None, debug: bool = False):
        self.player = Player(team_name)
        self.client = ZappyClient(host, port)

        self.host = host
        self.port = port

        self.is_genesis = is_genesis
        self.debug = debug
        self.map_dim = (0, 0)
        self.key = key

        self._ctx: BotContext | None = None
        self._fsm: FSM | None = None
        self._last_cmd: str | None = None

        signal.signal(signal.SIGCHLD, _reap_children)

    @staticmethod
    def launch_bot(team_name: str, host: str, port: int, is_genesis: bool, key: bytes | None = None, debug: bool = False) -> None:
        try:
            bot = ZappyAI(team_name, host, port, is_genesis, key, debug)
            bot.run()
        except KeyboardInterrupt:
            pass

    def run(self) -> None:
        import time
        connected = False
        retry_delay = 1.0

        while not connected:
            try:
                if self.key is not None:
                    set_encryption_key(self.key)

                _, w, h = self.client.connect_and_login(self.player.team_name)
                self.map_dim = (w, h)
                connected = True
            except Exception as e:
                if self.is_genesis:
                    print(f"[{self.player.team_name}] Genesis bot connection failed: {e}")
                    raise
                try:
                    self.client.close()
                except Exception:
                    pass
                time.sleep(retry_delay)
                self.client = ZappyClient(self.host, self.port)

        try:
            self._ctx = BotContext(
                player=self.player,
                client=self.client,
                map_dim=self.map_dim,
                team_name=self.player.team_name,
                host=self.host,
                port=self.port,
                key=self.key,
                debug_enabled=self.debug,
            )

            inv_resp = self._ctx.client.send_action(Commands.INVENTORY.build())
            self.player.update_inventory(inv_resp)
            self._ctx.debug(f"login ok map={self.map_dim} genesis={self.is_genesis}")

            if self.is_genesis:
                self._fsm = FSM(DiscoveryState(), self._ctx)
            else:
                self._ctx.is_queen = False
                self._ctx.role_decided = True
                from player.states.follower import FollowerMainState
                self._fsm = FSM(FollowerMainState(), self._ctx)

            self._main_loop()

        except ConnectionRefusedError:
            print(f"[{self.player.team_name}] Connection refused on {self.host}:{self.port}")
        except ConnectionAbortedError as e:
            self._debug_disconnect(f"aborted: {e}")
        except (ConnectionResetError, EOFError, BrokenPipeError):
            self._debug_disconnect("disconnected")
        except RuntimeError as e:
            if "closed" in str(e).lower() or "reset" in str(e).lower():
                print(f"[{self.player.team_name}] Server closed connection.")
            else:
                print(f"[{self.player.team_name}] Runtime Error: {e}")
        except KeyboardInterrupt:
            pass
        except Exception as e:
            import traceback
            print(f"[{self.player.team_name}] Critical failure: {e}")
            traceback.print_exc()
        finally:
            try:
                self.client.close()
            except Exception:
                pass

    def _debug_disconnect(self, reason: str) -> None:
        if self._ctx is not None:
            self._ctx.debug(f"STOP {reason} last_cmd={self._last_cmd}")
        else:
            print(f"[{self.player.team_name}] {reason}", flush=True)

    def _main_loop(self) -> None:
        while True:
            self.client.poll_events()
            self._check_events()
            self._tick()

    def _check_events(self) -> None:
        ctx = self._ctx

        while self.client.ejection_directions:
            self.client.ejection_directions.pop(0)

        while self.client.level_updates:
            new_lvl = self.client.level_updates.pop(0)
            if not ctx.is_queen:
                ctx.player.level = new_lvl
                ctx.elevation_target = None
                ctx.elevation_joined = False
                ctx.elevation_direction_fresh = False
                ctx.elevation_wait_ticks = 0
                ctx.action_queue.clear()
                setattr(ctx, "elevation_underway", False)
                ctx.debug(f"Level updated asynchronously to {new_lvl}")

        while self.client.elevation_underway_events:
            self.client.elevation_underway_events.pop(0)
            if not ctx.is_queen:
                setattr(ctx, "elevation_underway", True)
                ctx.debug("Elevation underway event received (frozen)")

        while self.client.incoming_broadcasts:
            direction, text = self.client.incoming_broadcasts.pop(0)
            msg = decode(self.player.team_name, text)
            if msg is None:
                continue
            self._handle_message(msg, direction)

    def _handle_message(self, msg: Message, direction: int = 0) -> None:
        ctx = self._ctx

        if msg.type == MsgType.PING:
            if ctx.is_queen:
                ctx.action_queue.appendleft(Commands.BROADCAST.build(
                    encode(ctx.team_name, MsgType.PONG, encode_pong())
                ))
            return

        if msg.type == MsgType.PONG:
            if not ctx.role_decided:
                ctx.is_queen = False
                ctx.role_decided = True
            return

        if msg.type == MsgType.POLL:
            if not ctx.is_queen:
                ctx.debug("recv POLL -> send ALIVE")
                ctx.client.send_action(Commands.BROADCAST.build(
                    encode(ctx.team_name, MsgType.ALIVE,
                           encode_alive(ctx.uuid, ctx.player.level, ctx.player.inventory))
                ))
            return

        if msg.type == MsgType.ALIVE:
            if ctx.is_queen:
                result = decode_alive(msg.payload)
                if result is not None:
                    uuid, level, inventory = result
                    ctx.register_follower_alive(uuid, level, inventory)
                    ctx.debug(
                        f"ALIVE from={uuid} lvl={level} food={inventory.get('food', '?')} "
                        f"alive={ctx.alive_follower_count()}"
                    )
            return

        if msg.type == MsgType.RESOURCE:
            result = decode_resource(msg.payload)
            if result is not None:
                resource, x, y = result
                ctx.known_resources[resource] = (x, y)
            return

        if msg.type == MsgType.INCANT:
            if ctx.is_queen:
                return

            result = decode_incant(msg.payload)
            if result is None:
                return

            level, missing, x, y = result

            if missing <= 0:
                return

            if level != ctx.player.level:
                return

            if not ctx.elevation_joined and ctx.elevation_target is None:
                if ctx.player.inventory.get("food", 0) < FOOD_INCANT:
                    ctx.debug(
                        f"ignore INCANT lvl={level} from_dir={direction}: "
                        f"food {ctx.player.inventory.get('food', 0)}/25"
                    )
                    return

            candidate_pos = (x, y)

            ctx.elevation_direction       = direction
            ctx.elevation_direction_fresh = True
            ctx.elevation_wait_ticks      = 0

            if ctx.elevation_target != candidate_pos:
                ctx.elevation_target = candidate_pos
                ctx.elevation_joined = False
                ctx.elevation_start_level = ctx.player.level
                ctx.action_queue.clear()
                ctx.debug(f"accept INCANT lvl={level} target={candidate_pos} dir={direction}")
            elif ctx.elevation_joined:
                ctx.elevation_wait_ticks = 0
                ctx.debug_every("joined_refresh", 12, "refresh joined incant target")

            return

        if msg.type == MsgType.JOIN:
            result = decode_join(msg.payload)
            if result is not None:
                uuid, level = result
                if level == ctx.player.level:
                    if ctx.is_queen:
                        ctx.queen_joined_followers.add(uuid)
                        ctx.debug(f"JOIN from {uuid} level={level}. Total joined={len(ctx.queen_joined_followers)}")
                    elif ctx.elevation_waiting > 0:
                        ctx.elevation_intents += 1
            return

        if msg.type == MsgType.DONE:
            result = decode_done(msg.payload)
            if result is None:
                return

            new_level, x, y = result

            if not ctx.is_queen:
                if ctx.elevation_joined or ctx.player.position == (x, y) or (ctx.elevation_target == (x, y) and ctx.elevation_joined):
                    ctx.player.level              = new_level
                    ctx.debug(f"DONE received: elevated to new_level={new_level}")
                else:
                    ctx.debug(f"DONE received but did not participate: pos={ctx.player.position} target={(x, y)} joined={ctx.elevation_joined}")
                
                if ctx.elevation_target == (x, y):
                    ctx.elevation_target          = None
                    ctx.elevation_joined          = False
                    ctx.elevation_direction_fresh = False
                    ctx.elevation_wait_ticks      = 0
                    ctx.action_queue.clear()
            return

    def _tick(self) -> None:
        ctx = self._ctx
        ctx.tick_counter += 1
        if ctx.is_queen:
            ctx.queen_tick += 1

        ctx.inventory_tick_counter += 1

        if ctx.inventory_tick_counter >= INVENTORY_CHECK_INTERVAL:
            ctx.inventory_tick_counter = 0
            inv_resp = ctx.client.send_action(Commands.INVENTORY.build())
            self.player.update_inventory(inv_resp)
            self._debug_inventory_change()

        self._fsm.tick(ctx)

        if not ctx.action_queue:
            ctx.action_queue.append(Commands.LOOK.build())

        cmd  = ctx.action_queue.popleft()
        self._last_cmd = cmd
        resp = ctx.client.send_action(cmd)
        self._apply_response(cmd, resp)
        self._debug_stationary(cmd, resp)

    def _debug_inventory_change(self) -> None:
        ctx = self._ctx
        inv = dict(self.player.inventory)

        if not ctx.debug_last_inventory:
            ctx.debug_last_inventory = inv
            return

        old_food = ctx.debug_last_inventory.get("food", 0)
        food = inv.get("food", 0)
        ctx.debug_last_inventory = inv

        if ctx.is_queen or food < FOOD_LOW or food <= old_food - 3:
            ctx.debug(f"inventory food {old_food}->{food} full={inv}")

    def _debug_stationary(self, cmd: str, resp: str) -> None:
        ctx = self._ctx
        pos = ctx.player.position

        if ctx.debug_last_position is None:
            ctx.debug_last_position = pos
            return

        if pos != ctx.debug_last_position:
            if ctx.debug_stationary_ticks >= 15:
                ctx.debug(
                    f"moved again after {ctx.debug_stationary_ticks} ticks "
                    f"from={ctx.debug_last_position} to={pos}"
                )
            ctx.debug_last_position = pos
            ctx.debug_stationary_ticks = 0
            return

        ctx.debug_stationary_ticks += 1
        if ctx.debug_stationary_ticks not in (15, 30) and ctx.debug_stationary_ticks % 50 != 0:
            return

        ctx.debug(
            "STATIONARY "
            f"ticks={ctx.debug_stationary_ticks} reason={self._stationary_reason()} "
            f"last_cmd={cmd} resp={resp} queue_head={self._queue_head()}"
        )

    def _stationary_reason(self) -> str:
        ctx = self._ctx
        state = self._state_name()
        food = ctx.player.inventory.get("food", 0)

        if ctx.is_queen:
            alive = ctx.alive_follower_count()
            expected = max(alive, ctx.queen_spawned_followers + ctx.queen_pending_fork_launches)
            return (
                f"state={state} food={food} alive={alive} "
                f"expected={expected}/{ctx.queen_max_followers} "
                f"forking={ctx.queen_is_forking} intents={ctx.elevation_intents}"
            )

        return (
            f"state={state} food={food} target={ctx.elevation_target} "
            f"joined={ctx.elevation_joined} fresh_dir={ctx.elevation_direction_fresh} "
            f"dir={ctx.elevation_direction}"
        )

    def _state_name(self) -> str:
        if self._fsm is None:
            return "none"

        state = self._fsm.current
        sub = getattr(state, "_sub", None)
        if sub is not None:
            return f"{state.__class__.__name__}/{sub.__class__.__name__}"
        return state.__class__.__name__

    def _queue_head(self) -> str:
        ctx = self._ctx
        return ctx.action_queue[0] if ctx.action_queue else "empty"

    def _apply_response(self, cmd: str, resp: str) -> None:
        ctx = self._ctx

        if not ctx.is_queen:
            if resp == "Elevation underway":
                setattr(ctx, "elevation_underway", True)
                ctx.debug("Received Elevation underway response (frozen)")
            elif resp.startswith("Current level:"):
                try:
                    lvl = int(resp.split(":")[1].strip())
                    ctx.player.level = lvl
                    ctx.elevation_target = None
                    ctx.elevation_joined = False
                    ctx.elevation_direction_fresh = False
                    ctx.elevation_wait_ticks = 0
                    ctx.action_queue.clear()
                    setattr(ctx, "elevation_underway", False)
                    ctx.debug(f"Level updated from response to {lvl}")
                except (IndexError, ValueError):
                    pass
            elif getattr(ctx, "elevation_underway", False):
                setattr(ctx, "elevation_underway", False)
                ctx.debug("Elevation no longer underway (received normal response)")

        if cmd == Commands.LOOK.build():
            ctx.player.update_vision(resp)
            ctx.player.map.update_with_look(ctx.player, ctx.player.vision, ctx.map_dim)

        elif cmd in (Commands.FORWARD.build(), Commands.RIGHT.build(), Commands.LEFT.build()):
            if resp == "ok":
                mov_enum = Movement[cmd.upper()]
                ctx.player._update_player_placement(mov_enum, ctx.map_dim)

        elif cmd == Commands.CONNECT_NBR.build():
            ctx.player.update_available_slot(resp)

        elif cmd == Commands.FORK.build():
            self._handle_fork_response(resp)

        elif cmd.startswith(f"{Commands.TAKE_OBJECT.value} ") and resp == "ok":
            obj = cmd.split(maxsplit=1)[1]
            if obj in ctx.player.inventory:
                ctx.player.inventory[obj] = ctx.player.inventory.get(obj, 0) + 1
                if ctx.is_queen or obj == "food" or ctx.player.inventory.get("food", 0) < FOOD_LOW:
                    ctx.debug(f"took {obj} local_qty={ctx.player.inventory.get(obj, 0)}")

        elif cmd.startswith(f"{Commands.SET_OBJECT.value} ") and resp == "ok":
            obj = cmd.split(maxsplit=1)[1]
            if obj in ctx.player.inventory:
                ctx.player.inventory[obj] = max(0, ctx.player.inventory.get(obj, 0) - 1)
                if ctx.is_queen or obj == "food" or ctx.player.inventory.get("food", 0) < FOOD_LOW:
                    ctx.debug(f"set {obj} local_qty={ctx.player.inventory.get(obj, 0)}")

    def _handle_fork_response(self, resp: str) -> None:
        ctx = self._ctx
        ctx.queen_is_forking = False

        if ctx.queen_pending_fork_launches > 0:
            ctx.queen_pending_fork_launches -= 1

        if resp != "ok":
            ctx.debug(f"fork failed resp={resp}")
            return

        ctx.queen_last_fork = ctx.queen_tick

        Process(
            name=f"AI_{ctx.team_name}_Forked",
            function=ZappyAI.launch_bot,
            args=[ctx.team_name, ctx.host, ctx.port, False, self.key, self.debug],
        ).start()
        ctx.queen_spawned_followers += 1
        ctx.debug(f"fork ok spawned={ctx.queen_spawned_followers}")
