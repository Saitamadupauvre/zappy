
import select
from socket import socket, AF_INET, SOCK_STREAM

class ClientSocket:
    sock: socket

    def __init__(self, sock: socket | None = None):
        if sock is None:
            self.sock = socket(AF_INET, SOCK_STREAM)
        else:
            self.sock = sock
        self._buffer = b""

    def connect(self, host: str, port: int) -> None:
        if not (1 <= port <= 65535):
            raise ValueError(f"Invalid port: {port}.")
        self.sock.connect((host, port))

    def send_text(self, message: str) -> None:
        if not message.endswith("\n"):
            message += "\n"
        self.sock.sendall(message.encode('utf-8'))

    def has_data(self, timeout: float = 0.0) -> bool:
        """Returns True if socket has data ready to read (non-blocking poll)."""
        ready, _, _ = select.select([self.sock], [], [], timeout)
        return bool(ready)

    def receive_line_nonblocking(self) -> str | None:
        """Returns a complete line if one is buffered, else reads available data.
        Returns None if no complete line available yet."""
        if b"\n" not in self._buffer:
            if self.has_data():
                chunk = self.sock.recv(4096)
                if not chunk:
                    raise ConnectionAbortedError("Server closed the connection.")
                self._buffer += chunk
        if b"\n" in self._buffer:
            line, self._buffer = self._buffer.split(b"\n", 1)
            return line.decode('utf-8')
        return None

    def receive_text(self) -> str:
        """Blocking read of one complete line."""
        while b"\n" not in self._buffer:
            ready, _, _ = select.select([self.sock], [], [])
            if ready:
                chunk = self.sock.recv(4096)
                if not chunk:
                    raise ConnectionAbortedError("Server closed the connection.")
                self._buffer += chunk
        line, self._buffer = self._buffer.split(b"\n", 1)
        return line.decode('utf-8')

    def fileno(self) -> int:
        return self.sock.fileno()

    def close(self) -> None:
        self.sock.close()

    def __enter__(self) -> "ClientSocket":
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        self.close()
# end of ClientSocket class
