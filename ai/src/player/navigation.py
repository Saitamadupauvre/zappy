from player.enum import Direction
from player.commands import Commands

def _shortest_delta(a: int, b: int, size: int) -> int:
    """Returns the shortest signed delta from a to b on a toroidal axis."""

    direct = (b - a) % size

    if direct <= size // 2: # operator '//' = Floor Division (5 // 2 = 2)
        return direct
    else:
        return direct - size

def _turns_to_face(current: Direction, target_dir: Direction) -> list[str]:
    """Returns the minimal turn commands to face target_dir from current."""

    diff = (target_dir.value - current.value) % 4
    if diff == Direction.NORTH.value:
        return []
    if diff == Direction.EAST.value:
        return [Commands.RIGHT.build()]
    if diff == Direction.WEST.value:
        return [Commands.LEFT.build()]

    return [Commands.RIGHT.build(), Commands.RIGHT.build()]

def navigate(
    start: tuple[int, int],
    direction: Direction,
    target: tuple[int, int],
    map_dim: tuple[int, int]
) -> list[str]:
    """
    Returns the sequence of commands to move from start to target.

    Args:
        start:      (x, y) current position
        direction:  current facing direction (0=North, 1=East, 2=South, 3=West)
        target:     (x, y) destination position
        map_dim:    (width, height) dimension of the toroidal map

    Returns:
        A list of commands, e.g. ["Forward", "Left", "Forward"]

    Notes:
        - North = y decreases, South = y increases
        - East  = x increases, West  = x decreases
        - Wrap-around is handled for both axes
    """

    sx, sy = start
    tx, ty = target
    map_width, map_height = map_dim

    commands: list[str] = []
    current_dir = direction

    dx = _shortest_delta(sx, tx, map_width)
    dy = _shortest_delta(sy, ty, map_height)

    # Move along X axis first, then Y axis
    for axis_delta, pos_dir, neg_dir in [
        (dx, Direction.EAST, Direction.WEST), # X: positive=East, negative=West
        (dy, Direction.SOUTH, Direction.NORTH), # Y: positive=South, negative=North
    ]:
        if axis_delta == 0:
            continue

        if axis_delta > 0:
            needed_dir = pos_dir
        else:
            needed_dir = neg_dir

        commands += _turns_to_face(current_dir, needed_dir)
        current_dir = needed_dir
        commands += [Commands.FORWARD.build()] * abs(axis_delta)

    return commands
