from enum import Enum

class Commands(Enum):
    FORWARD = "Forward"
    RIGHT = "Right"
    LEFT = "Left"
    LOOK = "Look"
    INVENTORY = "Inventory"
    BROADCAST = "Broadcast"
    CONNECT_NBR = "Connect_nbr"
    FORK = "Fork"
    EJECT = "Eject"
    TAKE_OBJECT = "Take"
    SET_OBJECT = "Set"
    INCANT = "Incantation"

    def build(self, arg: str = None) -> str:
        """Generate the string for the server as Commands.FORWARD.build() or
        Commands.TAKE_OBJECT.build(str)"""
        if arg is not None:
            return f"{self.value} {arg}"
        return self.value
