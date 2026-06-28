import csv
import os

CSV_HEADER = [
    "tick",
    "player_id",
    "level",
    "status",
    "linemate",
    "deraumere",
    "sibur",
    "mendiane",
    "phiras",
    "thystame",
    "chosen_action",
    "action_success",
]

class Logger:
    def __init__(self, filename: str):
        self.filename = filename

        if not os.path.exists(filename):
            with open(filename, "w", newline="") as f:
                csv.writer(f).writerow(CSV_HEADER)

    def log_simulation(
            self,
            tick: int,
            player_id: str | int,
            level: int,
            status: str,
            linemate: int,
            deraumere: int,
            sibur: int,
            mendiane: int,
            phiras: int,
            thystame: int,
            chosen_action: str,
            action_success: str,
    ) -> None:
        with open(self.filename, "a", newline="") as f:
            csv.writer(f).writerow(
                [
                    tick,
                    player_id,
                    level,
                    status,
                    linemate,
                    deraumere,
                    sibur,
                    mendiane,
                    phiras,
                    thystame,
                    chosen_action,
                    action_success,
                ]
            )