from unittest.mock import MagicMock, patch
from network.broadcast import MsgType, Message, encode_incant, encode_join, encode_resource
from player.zappyAI import ZappyAI
from player.context import BotContext
from player.player import Player
from player.fsm import FSM
from player.states.follower import FollowerMainState


def make_ai(level=1, position=(0, 0), map_dim=(10, 10)) -> ZappyAI:
    with patch("player.zappyAI.ZappyClient"), patch("player.zappyAI.signal"):
        ai = ZappyAI("alpha", "localhost", 4242, True)
    ai.map_dim = map_dim
    ai.player = Player("alpha")
    ai.player.level = level
    ai.player.position = position
    ai.player.inventory["food"] = 40
    ai.client = MagicMock()
    ai.client.send_action = MagicMock(return_value="ok")
    ai._ctx = BotContext(
        player=ai.player,
        client=ai.client,
        map_dim=map_dim,
        team_name="alpha",
    )
    ai._fsm = FSM(FollowerMainState(), ai._ctx)
    return ai


def test_incant_same_level_sets_elevation_target():
    ai = make_ai(level=2, position=(0, 0))
    ai.player.inventory["food"] = 40
    msg = Message("alpha", MsgType.INCANT, encode_incant(2, 1, (5, 5)))
    ai._handle_message(msg)
    assert ai._ctx.elevation_target == (5, 5)


def test_incant_wrong_level_ignored():
    ai = make_ai(level=1, position=(0, 0))
    msg = Message("alpha", MsgType.INCANT, encode_incant(3, 1, (5, 5)))
    ai._handle_message(msg)
    assert ai._ctx.elevation_target is None


def test_incant_missing_zero_ignored():
    ai = make_ai(level=2, position=(0, 0))
    msg = Message("alpha", MsgType.INCANT, encode_incant(2, 0, (5, 5)))
    ai._handle_message(msg)
    assert ai._ctx.elevation_target is None


def test_incant_malformed_ignored():
    ai = make_ai(level=2)
    msg = Message("alpha", MsgType.INCANT, "bad_payload")
    ai._handle_message(msg)
    assert ai._ctx.elevation_target is None


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


def test_resource_stored():
    ai = make_ai()
    msg = Message("alpha", MsgType.RESOURCE, encode_resource("linemate", 3, 4))
    ai._handle_message(msg)
    assert ai._ctx.known_resources["linemate"] == (3, 4)


def test_resource_malformed_ignored():
    ai = make_ai()
    msg = Message("alpha", MsgType.RESOURCE, "bad")
    ai._handle_message(msg)
    assert ai._ctx.known_resources == {}
