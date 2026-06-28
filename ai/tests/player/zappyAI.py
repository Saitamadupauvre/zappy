from unittest.mock import MagicMock, patch
from player.zappyAI import ZappyAI
from player.context import BotContext
from player.player import Player
from player.fsm import FSM
from player.states.follower import FollowerMainState
from network.broadcast import Message, MsgType, encode_incant, encode_join, encode_done


def make_ai(level: int = 1, position=(0, 0), food: int = 40) -> ZappyAI:
    with patch("player.zappyAI.ZappyClient"), patch("player.zappyAI.signal"):
        ai = ZappyAI("alpha", "localhost", 4242, True)
    ai.map_dim = (10, 10)
    ai.player = Player("alpha")
    ai.player.level = level
    ai.player.position = position
    ai.player.inventory["food"] = food
    ai.client = MagicMock()
    ai.client.send_action = MagicMock(return_value="ok")
    ai._ctx = BotContext(
        player=ai.player,
        client=ai.client,
        map_dim=ai.map_dim,
        team_name="alpha",
    )
    ai._fsm = FSM(FollowerMainState(), ai._ctx)
    return ai


# --- _handle_message INCANT ---

def test_incant_wrong_level_ignored():
    ai = make_ai(level=1)
    msg = Message("alpha", MsgType.INCANT, encode_incant(2, 1, (5, 5)))
    ai._handle_message(msg)
    assert ai._ctx.elevation_target is None

def test_incant_correct_level_sets_target():
    ai = make_ai(level=2)
    msg = Message("alpha", MsgType.INCANT, encode_incant(2, 1, (5, 5)))
    ai._handle_message(msg)
    assert ai._ctx.elevation_target == (5, 5)

def test_incant_missing_zero_ignored():
    ai = make_ai(level=2)
    msg = Message("alpha", MsgType.INCANT, encode_incant(2, 0, (5, 5)))
    ai._handle_message(msg)
    assert ai._ctx.elevation_target is None

def test_incant_malformed_ignored():
    ai = make_ai(level=2)
    msg = Message("alpha", MsgType.INCANT, "bad_payload")
    ai._handle_message(msg)
    assert ai._ctx.elevation_target is None


# --- _handle_message JOIN ---

def test_join_increments_intents():
    ai = make_ai(level=2)
    ai._ctx.elevation_waiting = 2
    msg = Message("alpha", MsgType.JOIN, encode_join("uuid-123", 2))
    ai._handle_message(msg)
    assert ai._ctx.elevation_intents == 1
    assert ai._ctx.elevation_waiting == 2

def test_join_wrong_level_ignored():
    ai = make_ai(level=2)
    ai._ctx.elevation_waiting = 2
    msg = Message("alpha", MsgType.JOIN, encode_join("uuid-123", 3))
    ai._handle_message(msg)
    assert ai._ctx.elevation_waiting == 2

def test_join_not_waiting_no_change():
    ai = make_ai(level=2)
    ai._ctx.elevation_waiting = 0
    msg = Message("alpha", MsgType.JOIN, encode_join("uuid-123", 2))
    ai._handle_message(msg)
    assert ai._ctx.elevation_waiting == 0


# --- _handle_message DONE ---

def test_done_clears_target_and_updates_level():
    ai = make_ai(level=2, position=(3, 4))
    ai._ctx.elevation_target = (3, 4)
    msg = Message("alpha", MsgType.DONE, encode_done(3, (3, 4)))
    ai._handle_message(msg)
    assert ai._ctx.elevation_target is None
    assert ai.player.level == 3

def test_done_wrong_pos_ignored():
    ai = make_ai(level=2, position=(3, 4))
    ai._ctx.elevation_target = (3, 4)
    msg = Message("alpha", MsgType.DONE, encode_done(3, (9, 9)))
    ai._handle_message(msg)
    assert ai._ctx.elevation_target == (3, 4)
    assert ai.player.level == 2


# --- FSM outer transitions ---

def test_follower_transitions_to_survive_on_low_food():
    from player.states.follower import _FS_Survive
    ai = make_ai(food=5)
    ai._fsm.tick(ai._ctx)
    assert isinstance(ai._fsm.current._sub, _FS_Survive)

def test_follower_transitions_to_gather_on_elevation_target():
    from player.states.follower import _FS_Gather
    ai = make_ai(food=30)
    ai._ctx.elevation_target = (5, 5)
    ai._fsm.tick(ai._ctx)
    assert isinstance(ai._fsm.current._sub, _FS_Gather)
