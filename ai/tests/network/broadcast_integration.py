
from unittest.mock import MagicMock, patch
from network.broadcast import (
    MsgType, Message,
    encode_incant,
    encode_join,
    encode_resource,
)
from player.player import State
from player.zappyAI import ZappyAI

def make_ai(team="alpha", level=1, position=(0, 0), direction=0, map_dim=(10, 10)) -> ZappyAI:
    """Creates a ZappyAI with a mocked client, no real socket."""
    with patch("player.zappyAI.ZappyClient"), patch("player.zappyAI.signal"):
        ai = ZappyAI(team, "localhost", 4242)
    ai.map_dim = map_dim
    ai.player.level = level
    ai.player.position = position
    ai.player.direction = direction
    ai.client = MagicMock()
    ai.client.send_action = MagicMock(return_value="ok")
    return ai

def test_incant_same_level_triggers_join_and_nav():
    ai = make_ai(level=2, position=(0, 0), direction=0)
    msg = Message("alpha", MsgType.INCANT, encode_incant(2, 1, (5, 5)))
    ai._handle_message(0, msg)

    assert ai.state == State.JOINING
    assert len(ai._nav_queue) > 0
    ai.client.send_action.assert_called_once()
    call_arg = ai.client.send_action.call_args[0][0]
    assert "JOIN" in call_arg

def test_incant_wrong_level_ignored():
    ai = make_ai(level=1, position=(0, 0), direction=0)
    msg = Message("alpha", MsgType.INCANT, encode_incant(3, 1, (5, 5)))
    ai._handle_message(0, msg)

    assert ai.state == State.IDLE
    assert ai._nav_queue == []
    ai.client.send_action.assert_not_called()

def test_incant_missing_zero_ignored():
    ai = make_ai(level=2, position=(0, 0), direction=0)
    msg = Message("alpha", MsgType.INCANT, encode_incant(2, 0, (5, 5)))
    ai._handle_message(0, msg)

    assert ai.state == State.IDLE
    assert ai._nav_queue == []

def test_incant_malformed_ignored():
    ai = make_ai(level=2)
    msg = Message("alpha", MsgType.INCANT, "bad_payload")
    ai._handle_message(0, msg)

    assert ai.state == State.IDLE
    ai.client.send_action.assert_not_called()

def test_incant_nav_queue_targets_correct_position():
    ai = make_ai(level=2, position=(0, 0), direction=0, map_dim=(10, 10))
    msg = Message("alpha", MsgType.INCANT, encode_incant(2, 1, (3, 0)))
    ai._handle_message(0, msg)

    # from (0,0) North to (3,0): turn Right then 3x Forward
    assert ai._nav_queue == ["Right", "Forward", "Forward", "Forward"]

def test_join_decrements_waiting_players():
    ai = make_ai(level=2)
    ai.state = State.WAITING
    ai._waiting_players = 2
    msg = Message("alpha", MsgType.JOIN, encode_join(2))
    ai._handle_message(0, msg)

    assert ai._waiting_players == 1

def test_join_wrong_level_ignored():
    ai = make_ai(level=2)
    ai.state = State.WAITING
    ai._waiting_players = 2
    msg = Message("alpha", MsgType.JOIN, encode_join(3))
    ai._handle_message(0, msg)

    assert ai._waiting_players == 2

def test_join_not_waiting_ignored():
    ai = make_ai(level=2)
    ai.state = State.IDLE
    ai._waiting_players = 2
    msg = Message("alpha", MsgType.JOIN, encode_join(2))
    ai._handle_message(0, msg)

    assert ai._waiting_players == 2

def test_resource_stored_in_known_resources():
    ai = make_ai()
    msg = Message("alpha", MsgType.RESOURCE, encode_resource("linemate", 3, 4))
    ai._handle_message(0, msg)

    assert ai.player.known_resources["linemate"] == (3, 4)

def test_resource_malformed_ignored():
    ai = make_ai()
    msg = Message("alpha", MsgType.RESOURCE, "bad")
    ai._handle_message(0, msg)

    assert ai.player.known_resources == {}

def test_move_forward_updates_position():
    ai = make_ai(position=(3, 3), direction=1)  # East
    ai._move("Forward")
    assert ai.player.position == (4, 3)

def test_move_right_updates_direction():
    ai = make_ai(direction=0)  # North
    ai._move("Right")
    assert ai.player.direction == 1  # East

def test_move_left_updates_direction():
    ai = make_ai(direction=0)  # North
    ai._move("Left")
    assert ai.player.direction == 3  # West

def test_move_no_update_on_ko():
    ai = make_ai(position=(3, 3), direction=0)
    ai.client.send_action = MagicMock(return_value="ko")
    ai._move("Forward")
    assert ai.player.position == (3, 3)
