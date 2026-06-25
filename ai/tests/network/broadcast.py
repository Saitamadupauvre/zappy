
from network.broadcast import (
    Message, MsgType,
    encode, decode,
    encode_incant, decode_incant,
    encode_join, decode_join,
    encode_pos, decode_pos,
    encode_resource, decode_resource,
)

def test_encode_incant():
    assert encode("alpha", MsgType.INCANT, encode_incant(2, 1, (3, 4))) == "alpha:INCANT:2:1:3,4"

def test_encode_pos():
    assert encode("alpha", MsgType.POS, encode_pos(5, 7)) == "alpha:POS:5,7"

def test_encode_resource():
    assert encode("alpha", MsgType.RESOURCE, encode_resource("linemate", 3, 4)) == "alpha:RESOURCE:linemate:3,4"

def test_encode_join():
    assert encode("alpha", MsgType.JOIN, encode_join(2)) == "alpha:JOIN:2"

def test_decode_valid():
    msg = decode("alpha", "alpha:INCANT:2:1:3,4")
    assert msg == Message(team_name="alpha", type=MsgType.INCANT, args="2:1:3,4")

def test_decode_wrong_team():
    assert decode("alpha", "beta:INCANT:2:1") is None

def test_decode_unknown_type():
    assert decode("alpha", "alpha:UNKNOWN:args") is None

def test_decode_malformed():
    assert decode("alpha", "malformed") is None

def test_decode_incant():
    assert decode_incant("2:1:3,4") == (2, 1, 3, 4)

def test_decode_incant_malformed():
    assert decode_incant("bad") is None

def test_decode_join():
    assert decode_join("2") == 2

def test_decode_join_malformed():
    assert decode_join("bad") is None

def test_decode_pos():
    assert decode_pos("5,7") == (5, 7)

def test_decode_pos_malformed():
    assert decode_pos("bad") is None

def test_decode_resource():
    assert decode_resource("linemate:3,4") == ("linemate", 3, 4)

def test_decode_resource_malformed():
    assert decode_resource("bad") is None

def test_roundtrip_incant():
    raw = encode("alpha", MsgType.INCANT, encode_incant(3, 2, (5, 7)))
    msg = decode("alpha", raw)
    assert decode_incant(msg.args) == (3, 2, 5, 7)

def test_roundtrip_resource():
    raw = encode("alpha", MsgType.RESOURCE, encode_resource("thystame", 9, 1))
    msg = decode("alpha", raw)
    assert decode_resource(msg.args) == ("thystame", 9, 1)
