
import signal
import os

from network.zappyClient import ZappyClient
from network.broadcast import (
    Message, MsgType,
    decode,
    decode_resource,
)
from player.comportement import Comportement
from player.commands import Commands
from player.process import Process
from player.player import Player
from player.enum import Movement

def _reap_children(signum, frame):
    """SIGCHLD handler: reap all terminated children to avoid zombies."""
    try:
        while True:
            pid, _ = os.waitpid(-1, os.WNOHANG)
            if pid == 0:
                break
    except ChildProcessError:
        pass


class ZappyAI:
    player: Player
    client: ZappyClient
    known_resources: dict[str, tuple[int, int]]

    host: str
    port: int
    is_genesis: bool

    map_dim: tuple[int, int]
    _nav_queue: list[str]
    _waiting_players: int

    def __init__(self, team_name: str, host: str, port: int, is_genesis: bool = False):
        self.player = Player(team_name)
        self.client = ZappyClient(host, port)
        self.known_resources = {}

        self.host = host
        self.port = port
        
        self.is_genesis = is_genesis
        self.map_dim = (0, 0)
        self._nav_queue = []
        self._waiting_players = 0

        self.comportement = Comportement()

        signal.signal(signal.SIGCHLD, _reap_children)

    @staticmethod
    def launch_bot(team_name: str, host: str, port: int, is_genesis: bool = False) -> None:
        try:
            bot = ZappyAI(team_name, host, port, is_genesis)
            bot.run()
        except KeyboardInterrupt:
            pass

    def run(self) -> None:
        try:
            slots, w, h = self.client.connect_and_login(self.player.team_name)
            self.map_dim = (w, h)

            if self.is_genesis:
                for i in range(slots):
                    brother = Process(
                        name=f"AI_{self.player.team_name}_Gen_{i}",
                        function=ZappyAI.launch_bot,
                        args=[self.player.team_name, self.host, self.port, False]
                    )
                    brother.start()

            self._main_loop()

        except ConnectionRefusedError:
            print(f"[{self.player.team_name}] Connection refused: Is the Zappy server running on {self.host}:{self.port} ?")

        except (ConnectionResetError, EOFError, BrokenPipeError, ConnectionAbortedError):
            print(f"[{self.player.team_name}] Disconnected: The ai died or the game ended.")
            
        except RuntimeError as e:
            if "closed" in str(e).lower() or "reset" in str(e).lower():
                print(f"[{self.player.team_name}] Disconnected: Server closed the connection.")
            else:
                print(f"[{self.player.team_name}] Runtime Error: {e}")

        except KeyboardInterrupt: # Ctrl+C
            pass

        except Exception as e:
            import traceback
            print(f"[{self.player.team_name}] Critical failure: {e}")
            traceback.print_exc()

        finally:
            try:
                self.client.close()
            except Exception:
                pass

    def _main_loop(self) -> None:
        while True:
            self.client.poll_events()
            self._check_events()
            self._think_and_act()

    def _check_events(self) -> None:
        while self.client.ejection_directions:
            dir_from = self.client.ejection_directions.pop(0)
            print(f"[{self.player.team_name}] Pushed from tile {dir_from}")

        while self.client.incoming_broadcasts:
            _, text = self.client.incoming_broadcasts.pop(0)
            msg = decode(self.player.team_name, text)

            if msg is None:
                continue

            self._handle_message(msg)

    def _handle_message(self, msg: Message) -> None:
        if msg.type == MsgType.INCANT:
            self.comportement.elevation.on_incant_received(msg.payload, self.player, self.client, self.map_dim)
        elif msg.type == MsgType.JOIN:
            self.comportement.elevation.on_join_received(msg.payload, self.player)
        elif msg.type == MsgType.DONE:
            self.comportement.elevation.on_done_received(msg.payload, self.player)
        elif msg.type == MsgType.RESOURCE:
            result = decode_resource(msg.payload)
            if result is None:
                return

            resource, x, y = result
            self.known_resources[resource] = (x, y)

    def _think_and_act(self) -> None:
        inv_response = self.client.send_action(Commands.INVENTORY.build())
        self.player.update_inventory(inv_response)

        self.comportement.update_player(self.player, self.map_dim, self.client)

        if self.comportement.action_queue:
            command = self.comportement.action_queue.pop(0)
            response = self.client.send_action(command)

            if command == Commands.LOOK.build():
                self.player.update_vision(response)
                self.player.map.update_with_look(self.player, self.player.vision, self.map_dim)

            elif command in [Commands.FORWARD.build(), Commands.RIGHT.build(), Commands.LEFT.build()] and response == "ok":
                mov_enum = Movement[command.upper()]
                self.player._update_player_placement(mov_enum, self.map_dim)
            return
# end of ZappyAI class
