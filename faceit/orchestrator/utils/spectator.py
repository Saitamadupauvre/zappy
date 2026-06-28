import asyncio


async def watch_for_winner(host: str, port: int, timeout: float = 3600.0) -> str:
    """
    Connect as a passive GUI client and wait for 'seg <team_name>\n'.
    Retries connection with backoff for up to 10s (server startup delay).
    Returns the winning team name.
    """
    reader, writer = await _connect_with_retry(host, port)
    try:
        async def _read_until_seg() -> str:
            while True:
                line = await reader.readline()
                if not line:
                    raise ConnectionError("Server closed connection before seg")
                decoded = line.decode(errors="replace").strip()
                if decoded.startswith("seg "):
                    return decoded.split(" ", 1)[1]

        return await asyncio.wait_for(_read_until_seg(), timeout=timeout)
    finally:
        writer.close()
        try:
            await writer.wait_closed()
        except Exception:
            pass


async def _connect_with_retry(host: str, port: int, retries: int = 20) -> tuple:
    for attempt in range(retries):
        try:
            reader, writer = await asyncio.open_connection(host, port)
            # handshake: wait for WELCOME, send GRAPHIC
            welcome = await reader.readline()
            if b"WELCOME" not in welcome:
                writer.close()
                raise ConnectionError("Unexpected handshake")
            writer.write(b"GRAPHIC\n")
            await writer.drain()
            return reader, writer
        except (ConnectionRefusedError, OSError):
            if attempt >= retries - 1:
                raise
            await asyncio.sleep(0.5 * (attempt + 1))
    raise ConnectionError("Could not connect to server")
