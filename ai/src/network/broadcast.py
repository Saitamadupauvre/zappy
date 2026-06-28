from network.encryptor import Encryptor
from dataclasses import dataclass
from enum import Enum

_encryptor = Encryptor()

def set_encryption_key(key: bytes) -> None:
    _encryptor.set_key(key)

class MsgType(Enum):
    PING     = "PING"     # Spawn -> all: "Is there a queen ?"
    PONG     = "PONG"     # Queen -> spawn: "I'm the queen, become a follower"

    ALIVE    = "ALIVE"    # Follower -> Queen: full status report (level + inventory)
    RESOURCE = "RESOURCE" # Follower -> Queen: "I saw a resource at (x,y)" # maybe remove

    POLL     = "POLL"     # Queen -> all: "envoyez-moi votre état" # a verifier
    ORDER    = "ORDER"    # Queen -> all: ordre générique (fork, collect…) # a verifier

    INCANT   = "INCANT"   # Queen -> all: gathering for incantation at my location
    JOIN     = "JOIN"     # Follower -> Queen: "I'm coming / I'm here"
    DONE     = "DONE"     # Queen -> all: "Incantation complete, new level"
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

    raw = f"{team_name}:{msg_type.value}:{payload}"
    return _encryptor.encrypt(raw)

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

    decrypted = _encryptor.decrypt(raw)
    if decrypted is None:
        return None

    parts = decrypted.split(":", 2)
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

def encode_ping() -> str:
    return ""

def encode_pong() -> str:
    return ""

_STONE_ORDER = ["food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]

def encode_alive(uuid: str, level: int, inventory: dict[str, int]) -> str:
    """
    > encode_alive("abc12345", 2, {"food": 10, "linemate": 1, ...})
    'abc12345:2:10:1:0:0:0:0:0'
    """

    parts = [uuid, str(level)] + [str(inventory.get(s, 0)) for s in _STONE_ORDER]
    return ":".join(map(str, parts))

def decode_alive(msg: str) -> tuple[str, int, dict[str, int]] | None:
    """
    Returns (uuid, level, inventory_dict) or None if malformed.
    """

    try:
        parts = msg.split(":")
        if len(parts) != 9:
            return None

        uuid = parts[0]
        level = int(parts[1])
        inventory = {s: int(parts[i + 2]) for i, s in enumerate(_STONE_ORDER)}

        return uuid, level, inventory

    except (ValueError, IndexError):
        return None

def encode_join(uuid: str, level: int) -> str:
    return f"{uuid}:{level}"

def decode_join(msg: str) -> tuple[str, int] | None:
    """Returns (uuid, level) or None if malformed."""
    try:
        parts = msg.split(":", 1)
        if len(parts) != 2:
            return None
        return parts[0], int(parts[1])
    except ValueError:
        return None

def encode_poll() -> str:
    return ""

def encode_order(action: str, arg: str = "") -> str:
    """
    > encode_order("fork")
    'fork'
    > encode_order("collect", "linemate")
    'collect:linemate'
    """

    if arg:
        return f"{action}:{arg}"

    return action

def decode_order(msg: str) -> tuple[str, str]:
    """
    Returns (action, arg). arg is "" if absent.
 
    > decode_order("collect:linemate")
    ('collect', 'linemate')
    > decode_order("fork")
    ('fork', '')
    """

    parts = msg.split(":", 1)
    action = parts[0]
    arg = parts[1] if len(parts) > 1 else ""
    return action, arg
 
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

def encode_done(level: int, position: tuple[int, int]) -> str:
    x, y = position
    return f"{level}:{x},{y}"

def decode_done(msg: str) -> tuple[int, int, int] | None:
    """Returns (new_level, x, y) or None if malformed."""

    try:
        level, coords = msg.split(":", 1)
        x, y = coords.split(",")
        return int(level), int(x), int(y)
    except ValueError:
        return None

def encode_resource(resource: str, x: int, y: int) -> str:
    return f"{resource}:{x},{y}"

def decode_resource(msg: str) -> tuple[str, int, int] | None:
    """Returns (resource, x, y) or None if malformed."""

    try:
        resource, coords = msg.split(":", 1)
        x, y = coords.split(",")
        return resource, int(x), int(y)
    except ValueError:
        return None
