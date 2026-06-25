from player.enum import Direction, Status, Movement
from player.map import Map
from player.vision import Vision, Tile

DEPLACEMENT = {
    Direction.NORTH: lambda x, y, w, h: (x, (y - 1) % h),
    Direction.EAST:  lambda x, y, w, h: ((x + 1) % w, y),
    Direction.SOUTH: lambda x, y, w, h: (x, (y + 1) % h),
    Direction.WEST:  lambda x, y, w, h: ((x - 1) % w, y)
}

class Player:
    """
    Tracks the internal state of a Zappy player, including its level,
    inventory, and survival metrics. It also validates elevation requirements.
    """

    TIME_UNITS = 126

    # Elevation static table
    # Format: target_level: {stone_name: required_quantity}
    ELEVATION_REQUIREMENTS = {
        2: {"linemate": 1, "deraumere": 0, "sibur": 0, "mendiane": 0, "phiras": 0, "thystame": 0},
        3: {"linemate": 1, "deraumere": 1, "sibur": 1, "mendiane": 0, "phiras": 0, "thystame": 0},
        4: {"linemate": 2, "deraumere": 0, "sibur": 1, "mendiane": 0, "phiras": 2, "thystame": 0},
        5: {"linemate": 1, "deraumere": 1, "sibur": 2, "mendiane": 0, "phiras": 1, "thystame": 0},
        6: {"linemate": 1, "deraumere": 2, "sibur": 1, "mendiane": 3, "phiras": 0, "thystame": 0},
        7: {"linemate": 1, "deraumere": 2, "sibur": 3, "mendiane": 0, "phiras": 1, "thystame": 0},
        8: {"linemate": 2, "deraumere": 2, "sibur": 2, "mendiane": 2, "phiras": 2, "thystame": 1},
    }

    team_name: str
    level: int

    inventory: dict[str, int]
    status: Status
    vision: Vision

    position: tuple[int, int]
    direction: Direction

    def __init__(self, team_name: str) -> None:
        self.team_name = team_name
        self.level = 1
        self.status = Status.SURVIVING
        self.map = Map()
        self.vision = None
        self.direction = Direction.NORTH
        self.position = (0, 0)
        
        self.inventory = {
            "food": 0,
            "linemate": 0,
            "deraumere": 0,
            "sibur": 0,
            "mendiane": 0,
            "phiras": 0,
            "thystame": 0
        }

    def update_inventory(self, server_response: str) -> None:
        """
        Parses the inventory string returned by the server and updates local state.
        
        Expected input format: "[food 345, sibur 3, phiras 5, deraumere 0]"
        """

        cleaned = server_response.strip("[] \n")
        tokens = cleaned.split(",")

        for token in tokens:
            match = token.strip().split()
            if len(match) == 2:
                try:
                    item_name, quantity = match[0], int(match[1])
                except ValueError:
                    continue
                if item_name in self.inventory:
                    self.inventory[item_name] = quantity

    def parse_look(self, server_response: str) -> Vision:
        """Parses the server response to the Look command into a Vision object.

        Expected format: "[player, food deraumere, , thystame, ...]"
        - Tiles are separated by commas
        - Objects on the same tile are separated by spaces
        - An empty tile is represented by an empty string between commas

        Returns:
            A list of dicts, one per visible tile.
            Each dict has the following keys, all integers:
                {
                    "player":    number of players on the tile,
                    "food":      number of food units,
                    "linemate":  number of linemate stones,
                    "deraumere": number of deraumere stones,
                    "sibur":     number of sibur stones,
                    "mendiane":  number of mendiane stones,
                    "phiras":    number of phiras stones,
                    "thystame":  number of thystame stones,
                }

        Example:
            > parse_look("[player, food linemate, , thystame]")
            [
                {"player": 1, "food": 0, "linemate": 0, ...}, # tile 0 (current)
                {"player": 0, "food": 1, "linemate": 1, ...}, # tile 1 (front-left)
                {"player": 0, "food": 0, "linemate": 0, ...}, # tile 2 (front-center)
                {"player": 0, "food": 0, "linemate": 0, "thystame": 1, ...}, # tile 3 (front-right)
            ]
        """

        cleaned = server_response.strip().strip("[]")
        raw_tiles = cleaned.split(",")
        tiles_list = []

        for raw in raw_tiles:
            tile_obj = Tile()

            for token in raw.strip().split():
                if token == "player":
                    tile_obj.players += 1
                elif hasattr(tile_obj, token):
                    current_qty = getattr(tile_obj, token)
                    setattr(tile_obj, token, current_qty + 1)

            tiles_list.append(tile_obj)

        return Vision(tiles_list, self.level)

    def update_vision(self, server_response: str) -> Vision:
        """
        Parses the look response and stores the current vision as a Vision object.
        """
        
        self.vision = self.parse_look(server_response)
        return self.vision

    def nearest_missing_stone(self) -> str | None:
        """
        Returns the name of the nearest missing stone needed for elevation,
        based on current vision and inventory.
        """

        next_level = self.level + 1
        if next_level not in self.ELEVATION_REQUIREMENTS:
            return None

        requirements = self.ELEVATION_REQUIREMENTS[next_level]
        missing_stones = []

        for stone, needed in requirements.items():
            if needed == 0:
                continue
            have = self.inventory.get(stone, 0)
            if have < needed:
                missing_stones.append(stone)

        if not missing_stones:
            return None

        if self.vision:
            for stone in missing_stones:
                if self.vision.find_nearest(stone) is not None:
                    return stone

        return missing_stones[0]

    def _update_player_placement(self, movement: Movement, map_dim: tuple[int, int]):
        """
        Update player coordinates/ direction depending on the wanted movement
        """

        if (movement == Movement.RIGHT):
            self.direction = Direction((self.direction.value + 1) % 4)
        if (movement == Movement.LEFT):
            self.direction = Direction((self.direction.value - 1) % 4)
        if (movement == Movement.FORWARD):
            w, h = map_dim
            self.position = DEPLACEMENT[self.direction](self.position[0], self.position[1], w, h)

    @property
    def time_to_live(self) -> int:
        """Returns the remaining time units based on the current food supply."""
        return self.inventory["food"] * self.TIME_UNITS
# end of Player class
