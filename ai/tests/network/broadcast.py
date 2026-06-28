from network.broadcast import (
    Message, MsgType,
    encode, decode,
    encode_incant, decode_incant,
    encode_join, decode_join,
    encode_resource, decode_resource,
    encode_alive, decode_alive,
    encode_done, decode_done,
)


def test_encode_incant():
    assert encode("alpha", MsgType.INCANT, encode_incant(2, 1, (3, 4))) == "alpha:INCANT:2:1:3,4"


def test_encode_resource():
    assert encode("alpha", MsgType.RESOURCE, encode_resource("linemate", 3, 4)) == "alpha:RESOURCE:linemate:3,4"


def test_encode_join():
    assert encode("alpha", MsgType.JOIN, encode_join("uuid-123", 2)) == "alpha:JOIN:uuid-123:2"


def test_decode_valid():
    msg = decode("alpha", "alpha:INCANT:2:1:3,4")
    assert msg == Message(team_name="alpha", type=MsgType.INCANT, payload="2:1:3,4")


def test_decode_wrong_team():
    assert decode("alpha", "beta:INCANT:2:1:3,4") is None


def test_decode_unknown_type():
    assert decode("alpha", "alpha:UNKNOWN:args") is None


def test_decode_malformed():
    assert decode("alpha", "malformed") is None


def test_decode_incant():
    assert decode_incant("2:1:3,4") == (2, 1, 3, 4)


def test_decode_incant_malformed():
    assert decode_incant("bad") is None


def test_decode_join():
    assert decode_join("uuid-123:2") == ("uuid-123", 2)


def test_decode_join_malformed():
    assert decode_join("bad") is None


def test_decode_resource():
    assert decode_resource("linemate:3,4") == ("linemate", 3, 4)


def test_decode_resource_malformed():
    assert decode_resource("bad") is None


def test_roundtrip_incant():
    raw = encode("alpha", MsgType.INCANT, encode_incant(3, 2, (5, 7)))
    msg = decode("alpha", raw)
    assert decode_incant(msg.payload) == (3, 2, 5, 7)


def test_roundtrip_resource():
    raw = encode("alpha", MsgType.RESOURCE, encode_resource("thystame", 9, 1))
    msg = decode("alpha", raw)
    assert decode_resource(msg.payload) == ("thystame", 9, 1)


def test_alive_roundtrip():
    inventory = {"food": 10, "linemate": 1, "deraumere": 0, "sibur": 0, "mendiane": 0, "phiras": 0, "thystame": 0}
    raw = encode("alpha", MsgType.ALIVE, encode_alive("uuid-123", 2, inventory))
    msg = decode("alpha", raw)
    decoded = decode_alive(msg.payload)
    assert decoded is not None
    uuid, level, inv = decoded
    assert uuid == "uuid-123"
    assert level == 2
    assert inv["food"] == 10
    assert inv["linemate"] == 1


def test_done_roundtrip():
    raw = encode("alpha", MsgType.DONE, encode_done(3, (5, 5)))
    msg = decode("alpha", raw)
    assert decode_done(msg.payload) == (3, 5, 5)
