from player.vision import Vision, Tile

class Map:
    """Map memory for players"""
    def __init__(self):
        self.grid: dict[tuple[int, int], Tile] = {}

    def update_with_look(self, player, vision: Vision, map_dim: tuple[int, int]):
        for tile_index, tile_content in enumerate(vision.tiles):
            abs_x, abs_y = Vision.get_xy_tile(tile_index, player, map_dim)
            self.grid[(abs_x, abs_y)] = tile_content
