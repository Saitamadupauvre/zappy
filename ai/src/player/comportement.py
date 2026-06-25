from dataclasses import dataclass, field
from player.navigation import navigate
from network.zappyClient import ZappyClient
from player.elevation import Elevation
from player.player import Player
from player.vision import Vision
from player.commands import Commands
from player.enum import Status
import random

@dataclass
class Comportement:
    """
    Class repertoring every possible comportement of the player depending
    of his current state
    """

    FOOD_LOW = 15     # under: survive mode
    FOOD_HIGH = 30    # on: can stop survive mode
    FOOD_CRITICAL = 5 # imminent death: stop all and survive

    action_queue: list[str] = field(default_factory=list)
    elevation: Elevation = field(default_factory=Elevation)

    def update_player(self, player: Player, map_dim: tuple[int, int], client=None):
        """
        Update player behavior from his states
        """

        food_count = player.inventory.get("food", 0)

        if player.status == Status.ELEVATING:
            if food_count < self.FOOD_CRITICAL:
                player.status = Status.SURVIVING
                self.elevation.is_leader = False
                self.elevation.is_candidate = False
                self.elevation._nav_queue.clear()
                self.elevation._target_leader_pos = None
            else:
                if self.action_queue:
                    return
                self.elevation.update(player, client, map_dim)
            return

        if food_count < self.FOOD_LOW:
            player.status = Status.SURVIVING
            self.elevation.is_leader = False
            self.elevation.is_candidate = False
            self.elevation._nav_queue.clear()
            self.elevation._target_leader_pos = None
        elif player.status == Status.SURVIVING and food_count >= self.FOOD_HIGH:
            player.status = Status.COLLECTING
            self.elevation.is_leader = False
            self.elevation.is_candidate = False

        if self.action_queue:
            return

        if player.status == Status.SURVIVING:
            self._handle_surviving(player, map_dim)
        elif player.status == Status.COLLECTING:
            self._handle_collect_stone(player, client, map_dim)

    def _handle_surviving(self, player: Player, map_dim: tuple[int, int]):
        """
        Surviving by taking food on the map 
        """

        if player.vision is None:
            self.action_queue.append(Commands.LOOK.build())
            return

        tile_index = player.vision.find_nearest("food")
        
        if tile_index is None:
            turn = random.choice([Commands.LEFT.build(), Commands.RIGHT.build(), ""])
            if turn:
                self.action_queue.append(turn)

            self.action_queue.append(Commands.FORWARD.build())
            self.action_queue.append(Commands.LOOK.build())
            return

        target_xy = Vision.get_xy_tile(tile_index, player, map_dim)
        
        path_commands = navigate(player.position, player.direction, target_xy, map_dim)
        
        self.action_queue.extend(path_commands)
        self.action_queue.append(Commands.TAKE_OBJECT.build("food"))
        self.action_queue.append(Commands.LOOK.build())

    def _handle_collect_stone(self, player: Player, client: ZappyClient, map_dim: tuple[int, int]):
        """
        Collecting stones for incantations, if the player has the required stones 
        his status become ELEVATING
        """

        if player.vision is None:
            self.action_queue.append(Commands.LOOK.build())
            return

        next_level = player.level + 1
        requirements = player.ELEVATION_REQUIREMENTS.get(next_level, {})
        
        has_all_stones = True
        for stone, qty_needed in requirements.items():
            if player.inventory.get(stone, 0) < qty_needed:
                has_all_stones = False
                break

        if has_all_stones:
            player.status = Status.ELEVATING
            self.elevation.become_candidate(player, client, map_dim)
            return

        needed_stone = player.nearest_missing_stone()
        tile_index = player.vision.find_nearest(needed_stone) if needed_stone else None
        
        if tile_index is None:
            turn = random.choice([Commands.LEFT.build(), Commands.RIGHT.build(), ""])
            if turn:
                self.action_queue.append(turn)

            self.action_queue.append(Commands.FORWARD.build())
            self.action_queue.append(Commands.LOOK.build())
            return

        target_xy = Vision.get_xy_tile(tile_index, player, map_dim)
        
        path_commands = navigate(player.position, player.direction, target_xy, map_dim)
        
        self.action_queue.extend(path_commands)
        self.action_queue.append(Commands.TAKE_OBJECT.build(needed_stone))
        self.action_queue.append(Commands.LOOK.build())
