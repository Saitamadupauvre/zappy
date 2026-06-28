from enum import Enum

class Direction(Enum):
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3

class Movement(Enum):
    FORWARD = 0
    RIGHT = 1
    LEFT = 2
