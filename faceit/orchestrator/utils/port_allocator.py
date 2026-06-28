import asyncio
import socket


async def find_free_port(start: int = 4300, end: int = 5000) -> int:
    loop = asyncio.get_event_loop()
    for port in range(start, end):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            await loop.run_in_executor(None, sock.bind, ("", port))
            sock.close()
            return port
        except OSError:
            continue
    raise RuntimeError("No free port found")
