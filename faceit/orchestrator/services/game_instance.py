import asyncio
import logging
from datetime import datetime

from sqlalchemy.ext.asyncio import AsyncSession

from ..config import settings
from ..database import AsyncSessionLocal
from ..models.ai_binary import AIBinary
from ..models.match import Match, MatchParticipant
from ..models.room import Room
from ..models.server_binary import ServerBinary
from ..utils.port_allocator import find_free_port
from ..utils.spectator import watch_for_winner
from . import mmr_engine

log = logging.getLogger(__name__)


class GameInstance:
    def __init__(self, match_id: int, participants: list[MatchParticipant], room: Room):
        self.match_id = match_id
        self.participants = participants
        self.room = room

    async def run(self) -> None:
        async with AsyncSessionLocal() as session:
            try:
                await self._run(session)
            except Exception as exc:
                log.exception("GameInstance %d failed: %s", self.match_id, exc)
                await self._set_error(session)

    async def _run(self, session: AsyncSession) -> None:
        port = await find_free_port()

        match = await session.get(Match, self.match_id)
        match.server_port = port
        match.status = "running"
        match.started_at = datetime.utcnow()
        session.add(match)
        await session.commit()

        team_names = [p.team_name for p in self.participants]
        server_proc = await self._spawn_server(port, team_names, session)

        ai_procs = await self._spawn_ais(port, session)

        try:
            winner = await watch_for_winner("localhost", port, timeout=settings.match_timeout)
        except Exception as exc:
            log.error("Match %d spectator error: %s", self.match_id, exc)
            winner = None
        finally:
            for proc in ai_procs:
                try:
                    proc.terminate()
                    await asyncio.wait_for(proc.wait(), timeout=5)
                except Exception:
                    pass
            try:
                server_proc.terminate()
                await asyncio.wait_for(server_proc.wait(), timeout=5)
            except Exception:
                pass

        room = await session.get(Room, self.room.id)
        if room:
            room.status = "closed"
            session.add(room)

        if winner:
            await mmr_engine.update_match(self.match_id, winner, session)
        else:
            await self._set_error(session)

    async def _spawn_server(self, port: int, team_names: list[str], session: AsyncSession):
        srv = await session.get(ServerBinary, self.room.server_binary_id)
        if not srv:
            raise RuntimeError(f"ServerBinary {self.room.server_binary_id} not found")

        cmd = [
            srv.file_path,
            "-p", str(port),
            "-x", str(settings.map_width),
            "-y", str(settings.map_height),
            "-n", *team_names,
            "-c", "1",
            "-f", str(settings.freq),
        ]
        log.info("Spawning server: %s", " ".join(cmd))
        return await asyncio.create_subprocess_exec(
            *cmd,
            stdout=asyncio.subprocess.DEVNULL,
            stderr=asyncio.subprocess.DEVNULL,
        )

    async def _spawn_ais(self, port: int, session: AsyncSession) -> list:
        procs = []
        for p in self.participants:
            ai = await session.get(AIBinary, p.ai_binary_id)
            if not ai:
                continue
            cmd = [ai.file_path, "-p", str(port), "-h", "localhost", "-n", p.team_name]
            log.info("Spawning AI %s: %s", p.team_name, " ".join(cmd))
            proc = await asyncio.create_subprocess_exec(
                *cmd,
                stdout=asyncio.subprocess.DEVNULL,
                stderr=asyncio.subprocess.DEVNULL,
            )
            procs.append(proc)
        return procs

    async def _set_error(self, session: AsyncSession) -> None:
        match = await session.get(Match, self.match_id)
        if match:
            match.status = "error"
            match.finished_at = datetime.utcnow()
            session.add(match)
        room = await session.get(Room, self.room.id)
        if room:
            room.status = "closed"
            session.add(room)
        await session.commit()
