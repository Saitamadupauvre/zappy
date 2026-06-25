
from unittest.mock import MagicMock, patch
from player.zappyAI import ZappyAI, State
from network.broadcast import Message, MsgType, encode_incant, encode_join

def make_ai(level: int = 1) -> ZappyAI:
    ai = ZappyAI.__new__(ZappyAI)
    ai.player = MagicMock()
    ai.player.team_name = "alpha"
    ai.player.level = level
    ai.player.position = (0, 0)
    ai.player.direction = 0
    ai.player.inventory = {"food": 10}
    ai.player.vision = [{"player": 0}]
    ai.client = MagicMock()
    ai.state = State.IDLE
    ai.map_dim = (10, 10)
    ai._nav_queue = []
    ai._waiting_players = 0
    return ai

def test_handle_incant_wrong_level():
    ai = make_ai(level=1)
    msg = Message(team_name="alpha", type=MsgType.INCANT, payload=encode_incant(2, 1, (5, 5)))
    ai._handle_message(msg)
    assert ai.state == State.IDLE
    assert ai._nav_queue == []

def test_handle_incant_correct_level():
    ai = make_ai(level=2)
    msg = Message(team_name="alpha", type=MsgType.INCANT, payload=encode_incant(2, 1, (5, 5)))
    print(f"msg.type = {msg.type}")
    print(f"MsgType.INCANT = {MsgType.INCANT}")
    print(f"equal = {msg.type == MsgType.INCANT}")
    ai._handle_message(msg)
    assert ai.state == State.JOINING

def test_handle_join_decrements_waiting():
    ai = make_ai(level=2)
    ai.state = State.WAITING
    ai._waiting_players = 2
    msg = Message(team_name="alpha", type=MsgType.JOIN, payload=encode_join(2))
    ai._handle_message(msg)
    assert ai._waiting_players == 1

def test_handle_join_wrong_level():
    ai = make_ai(level=2)
    ai.state = State.WAITING
    ai._waiting_players = 2
    msg = Message(team_name="alpha", type=MsgType.JOIN, payload=encode_join(3))
    ai._handle_message(msg)
    assert ai._waiting_players == 2

def test_enough_players_sets_idle():
    ai = make_ai(level=1)
    ai.player.vision = [{"player": 1}]
    ai.player.level = 1
    ai.client.send_action.return_value = "[player,,]"
    ai.player.update_vision = MagicMock(side_effect=lambda _: None)

    with patch("player.zappyAI.PLAYERS_REQUIRED", {2: 1}):
        ai._look_and_evaluate()

    assert ai.state == State.IDLE

def test_not_enough_players_rebroadcasts():
    ai = make_ai(level=1)
    ai.player.vision = [{"player": 0}]
    ai.player.level = 1
    ai.client.send_action.return_value = "[,,]"
    ai.player.update_vision = MagicMock(side_effect=lambda _: None)

    with patch("player.zappyAI.PLAYERS_REQUIRED", {2: 1}):
        ai._look_and_evaluate()

    assert ai.state == State.WAITING
    assert ai._waiting_players == 1
