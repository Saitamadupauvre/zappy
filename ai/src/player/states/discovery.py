from __future__ import annotations
from player.fsm import State
from player.context import BotContext
from player.commands import Commands
from network.broadcast import MsgType, encode, encode_ping
from player.states.follower import FollowerMainState

PING_TIMEOUT = 25

class DiscoveryState(State):
    def enter(self, ctx: BotContext) -> None:
        ctx.role_decided = False
        ctx.ping_tick = 0
        ctx.action_queue.appendleft(Commands.BROADCAST.build(
            encode(ctx.team_name, MsgType.PING, encode_ping())
        ))

    def update(self, ctx: BotContext) -> State | None:
        if ctx.role_decided:
            return self._pick_role(ctx)

        if ctx.ping_tick >= PING_TIMEOUT:
            ctx.is_queen = True
            ctx.role_decided = True
            return self._pick_role(ctx)

        ctx.action_queue.appendleft(Commands.INVENTORY.build())
        ctx.ping_tick += 1

        return None

    def exit(self, ctx: BotContext) -> None:
        pass

    @staticmethod
    def _pick_role(ctx: BotContext) -> State:
        if ctx.is_queen:
            from player.queen.state import QueenState
            return QueenState()
        else:
            return FollowerMainState()
