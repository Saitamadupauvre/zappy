import time
import select
from collections import deque
from network.socket import ClientSocket

MAX_PENDING = 10

class ZappyClient:
    network: ClientSocket
    incoming_broadcasts: list[tuple[int, str]]
    ejection_directions: list[int]
    level_updates: list[int]
    elevation_underway_events: list[bool]
    _pending: deque

    def __init__(self, host: str, port: int) -> None:
        self.network = ClientSocket()
        self.host = host
        self.port = port
        self.incoming_broadcasts = []
        self.ejection_directions = []
        self.level_updates = []
        self.elevation_underway_events = []
        self._pending = deque()

    def connect_and_login(self, team_name: str) -> tuple[int, int, int]:
        """Handshake with server. Returns (slots, map_width, map_height)."""
        self.network.connect(self.host, self.port)

        if self.network.receive_text() != "WELCOME":
            raise RuntimeError("Expected WELCOME from server")

        self.network.send_text(team_name)

        slots_line = self.network.receive_text()
        if slots_line == "ko":
            raise RuntimeError(f"Server rejected team '{team_name}'")

        w, h = self.network.receive_text().split()
        return int(slots_line), int(w), int(h)

    def close(self) -> None:
        self.network.close()

    def send_command(self, command: str) -> None:
        """Send a command. Raises if the 10-command in-flight limit is reached."""
        if len(self._pending) >= MAX_PENDING:
            raise RuntimeError(f"Command queue full ({MAX_PENDING} in-flight limit)")
        self.network.send_text(command)
        self._pending.append(command)

    def send_action(self, command: str) -> str:
        """Send a command and block until its response arrives."""
        self.send_command(command)
        return self.get_response()

    def get_response(self) -> str:
        """
        Block until the server responds to the oldest pending command.
        Any interleaved async events (eject / broadcast) are buffered.
        """
        if not self._pending:
            raise RuntimeError("No pending command")

        pending_incant = any(cmd.startswith("Incantation") for cmd in self._pending)

        while True:
            if not self.network.has_buffered_line():
                select.select([self.network], [], [])
            line = self.network.receive_text()
            if line == "dead":
                raise ConnectionAbortedError("Player is dead")
            elif _is_event(line, pending_incant):
                self._store_event(line)
            else:
                self._pending.popleft()
                return line

    def poll_events(self) -> None:
        """Drain all immediately available async events without blocking."""
        pending_incant = any(cmd.startswith("Incantation") for cmd in self._pending)
        while self.network.has_data():
            line = self.network.receive_line_nonblocking()
            if line is None:
                break
            if _is_event(line, pending_incant):
                self._store_event(line)

    def _store_event(self, line: str) -> None:
        if line.startswith("eject:"):
            self._parse_eject(line)
        elif line.startswith("message "):
            self._parse_broadcast(line)
        elif line.startswith("Current level:"):
            try:
                lvl = int(line.split(":")[1].strip())
                self.level_updates.append(lvl)
            except (IndexError, ValueError):
                pass
        elif line.startswith("Elevation underway"):
            self.elevation_underway_events.append(True)

    def _parse_eject(self, line: str) -> None:
        try:
            direction = int(line.split(":")[1].strip())
            self.ejection_directions.append(direction)
        except (IndexError, ValueError):
            pass

    def _parse_broadcast(self, line: str) -> None:
        try:
            header, text = line.split(",", 1)
            direction = int(header.split()[1])
            self.incoming_broadcasts.append((direction, text.strip()))
        except (IndexError, ValueError):
            pass
    
    def wait_for_incantation_result(self) -> str:
        while True:
            if not self.network.has_buffered_line():
                select.select([self.network], [], [])
            line = self.network.receive_text()
            if line == "dead":
                raise ConnectionAbortedError("Player is dead")
            elif _is_event(line, pending_incant=True):
                self._store_event(line)
            else:
                return line
# end of ZappyClient class

def _is_event(line: str, pending_incant: bool = False) -> bool:
    if line.startswith("eject:") or line.startswith("message "):
        return True
    if line.startswith("Current level:") and not pending_incant:
        return True
    if line.startswith("Elevation underway") and not pending_incant:
        return True
    return False