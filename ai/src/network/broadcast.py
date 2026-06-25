
from dataclasses import dataclass
from enum import Enum

class MsgType(Enum):
    INCANT   = "INCANT"   # need X players for incantation at my level
    JOIN     = "JOIN"     # i'm coming to join an incantation
    POS      = "POS"      # broadcasting my current position
    RESOURCE = "RESOURCE" # i spotted a resource at (x, y)
    DONE     = "DONE"     # incantation finished (for the followers)
# End of MsgType enum

@dataclass
class Message:
    team_name: str
    type: MsgType
    payload: str
# End of Message class

def encode(team_name: str, msg_type: MsgType, payload: str) -> str:
    """
    Encodes a message to send via Broadcast.

    > encode("alpha", MsgType.INCANT, "2:1")
    'alpha:INCANT:2:1'
    """

    return f"{team_name}:{msg_type.value}:{payload}"

def decode(team_name: str, raw: str) -> Message | None:
    """
    Decodes a raw broadcast string into a Message.
    Returns None if the message is malformed or from another team.

    Expected format: "TEAM:TYPE:payload"

    > decode("alpha", "alpha:INCANT:2:1")
    Message(team='alpha', type=<MsgType.INCANT: 'INCANT'>, payload='2:1')

    > decode("alpha", "beta:INCANT:2:1")
    None  # ignored, different team
    """

    parts = raw.split(":", 2)

    if len(parts) != 3:
        return None
    
    team_sender, msg_type_str, payload = parts
    if team_sender != team_name:
        return None # not our team
    
    try:
        msg_type = MsgType(msg_type_str)
    except ValueError:
        return None # unknown type
    
    return Message(team_sender, msg_type, payload)

def encode_incant(level: int, missing_players: int, position: tuple[int, int]) -> str:
    x, y = position
    return f"{level}:{missing_players}:{x},{y}"

def decode_incant(msg: str) -> tuple[int, int, int, int] | None:
    """Returns (level, missing_players, x, y) or None if malformed."""
    
    try:
        level, missing, coords = msg.split(":", 2)
        x, y = coords.split(",")
        return int(level), int(missing), int(x), int(y)
    except ValueError:
        return None
    
def encode_join(level: int) -> str:
    return str(level)

def decode_join(msg: str) -> int | None:
    """Returns level or None if malformed."""

    try:
        return int(msg)
    except ValueError:
        return None

def encode_pos(x: int, y: int) -> str:
    return f"{x},{y}"

def decode_pos(msg: str) -> tuple[int, int] | None:
    """Returns position tuple(x, y) or None if malformed."""

    try:
        x, y = msg.split(",")
        return int(x), int(y)
    except ValueError:
        return None

def encode_resource(resource: str, x: int, y: int) -> str:
    return f"{resource}:{x},{y}"

def decode_resource(msg: str) -> tuple[str, int, int] | None:
    """Returns ressource name and position tuple(name, x, y) or None if malformed."""

    try:
        resource, coords = msg.split(":", 1)
        x, y = coords.split(",")
        return resource, int(x), int(y)
    except ValueError:
        return None

def encode_done(level: int, position: tuple[int, int]) -> str:
    x, y = position
    return f"{level}:{x},{y}"

def decode_done(msg: str) -> tuple[int, int, int] | None:
    try:
        level, coords = msg.split(":", 1)
        x, y = coords.split(",")
        return int(level), int(x), int(y)
    except ValueError:
        return None
