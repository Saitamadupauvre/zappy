from __future__ import annotations
from dataclasses import dataclass, field
from player.enum import Direction
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from player.player import Player

"""Represents the difference 'map position' for each possible field of vision"""
POV_CORRESPONDANCE = {
    0: (0, 0),
    1: (1, -1), 2: (1, 0), 3: (1, 1),
    4: (2, -2), 5: (2, -1), 6: (2, 0), 7: (2, 1), 8: (2, 2),
    9: (3, -3), 10: (3, -2), 11: (3, -1), 12: (3, 0), 13: (3, 1), 14: (3, 2), 15: (3, 3)
}

@dataclass
class Tile:
    """Represents the content of a single tile from the look command."""
    players: int = 0
    food: int = 0
    linemate: int = 0
    deraumere: int = 0
    sibur: int = 0
    mendiane: int = 0
    phiras: int = 0
    thystame: int = 0

    RESOURCES = {"food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"}

    def has_resources(self) -> bool:
        for r in self.RESOURCES:
            if getattr(self, r) > 0:
                return True

    def total_stones(self) -> int:
        total: int = 0

        for r in self.RESOURCES:
            if getattr(self, r) and r != "food":
                total += 1
        return total
# end of Tile class

@dataclass
class Vision:
    """
    Structured representation of the look command response.

    The tiles are indexed as per the protocol:
      - tiles[0] -> current tile (player's position)
      - tiles[1..3] -> row in front at level 1
      - tiles[4..8] -> row in front at level 2
      - etc.

    At level N, the player sees (2N+1) tiles per row, for N+1 rows total.
    """

    tiles: list[Tile] = field(default_factory=list)
    level: int = 1

    def get_row(self, row_index: int) -> list[Tile]:
        """
        Returns all tiles of a given row (0 = current tile row).
        Row 0 -> [tiles[0]]
        Row 1 -> [tiles[1], tiles[2], tiles[3]]
        Row k -> 2k+1 tiles
        """

        if row_index == 0:
            return [self.tiles[0]]

        start = row_index ** 2 # 0, 1, 4, 9 (left of the row k²)
        width = 2 * row_index + 1 # 1, 3, 5, 7 (nbr of tiles on the row k)

        return self.tiles[start:start + width]

    @staticmethod
    def get_xy_tile(tile_index: int, player: Player, map_dim: tuple[int, int]) -> tuple[int, int]:
        """
        return the translated tile index to XY position with the player direction
        """
        row = 0
        index_row = 0
        for r in range(player.level + 1):
            if (r ** 2) <= tile_index < ((r ** 2) + (2 * r + 1)):
                row = r
                index_row = tile_index - (r ** 2)
                break
        colonne = index_row - row

        if player.direction == Direction.NORTH:
            dx, dy = colonne, -row
        elif player.direction == Direction.EAST:
            dx, dy = row, colonne
        elif player.direction == Direction.SOUTH:
            dx, dy = -colonne, row
        else: # WEST
            dx, dy = -row, -colonne

        w, h = map_dim
        target_x = (player.position[0] + dx) % w
        target_y = (player.position[1] + dy) % h

        return (target_x, target_y)

    def find_nearest(self, resource: str) -> int | None:
        """Return the nearest indice that contain the given resource."""
        for i, tile in enumerate(self.tiles):
            if resource == "food" and tile.food > 0:
                return i
            elif resource == "player" and tile.players > 0:
                return i
            elif getattr(tile, resource, 0) > 0:
                return i
        return None

    def find_all(self, resource: str) -> list[int]:
        """Returns all tile indices that contain the given resource."""
        result = []
        for i, tile in enumerate(self.tiles):
            if resource == "player":
                if tile.players > 0:
                    result.append(i)
            elif getattr(tile, resource, 0) > 0:
                result.append(i)
        return result
# end of Vision class
