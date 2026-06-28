import asyncio
import logging
import re
from dataclasses import dataclass, field
from datetime import datetime

from sqlalchemy.ext.asyncio import AsyncSession

from ..models.ai_binary import AIBinary
from ..models.match import Match, MatchParticipant
from ..models.room import Room
from .game_instance import GameInstance

log = logging.getLogger(__name__)


@dataclass
class QueueEntry:
    ai_binary_id: int
    user_id: int
    ai_name: str
    room_id: int
    joined_at: datetime = field(default_factory=datetime.utcnow)


class MatchmakerService:
    def __init__(self) -> None:
        # room_id -> list of QueueEntry
        self._queues: dict[int, list[QueueEntry]] = {}
        self._lock = asyncio.Lock()

    async def enqueue(self, entry: QueueEntry, session: AsyncSession) -> dict:
        async with self._lock:
            room = await session.get(Room, entry.room_id)
            if not room or room.status != "open":
                return {"error": "Room not open"}

            queue = self._queues.setdefault(entry.room_id, [])

            if any(e.ai_binary_id == entry.ai_binary_id for q in self._queues.values() for e in q):
                return {"error": "AI already queued"}

            queue.append(entry)
            count = len(queue)
            log.info("Room %d: AI %d joined (%d/4)", entry.room_id, entry.ai_binary_id, count)

            if count >= room.max_players:
                await self._trigger(entry.room_id, session)

            return {"room_id": entry.room_id, "players": count}

    async def dequeue(self, ai_binary_id: int) -> bool:
        async with self._lock:
            for queue in self._queues.values():
                before = len(queue)
                queue[:] = [e for e in queue if e.ai_binary_id != ai_binary_id]
                if len(queue) < before:
                    return True
            return False

    def room_status(self, room_id: int) -> dict:
        queue = self._queues.get(room_id, [])
        return {"room_id": room_id, "players": len(queue), "max": 4}

    def room_entries(self, room_id: int) -> list[QueueEntry]:
        return list(self._queues.get(room_id, []))

    async def _trigger(self, room_id: int, session: AsyncSession) -> None:
        room = await session.get(Room, room_id)
        max_p = room.max_players if room else 4
        entries = self._queues.pop(room_id, [])[:max_p]

        match = Match(map_width=10, map_height=10, freq=100)
        session.add(match)
        await session.commit()
        await session.refresh(match)
        room.status = "running"
        room.match_id = match.id
        session.add(room)

        participants: list[MatchParticipant] = []
        for i, entry in enumerate(entries):
            ai = await session.get(AIBinary, entry.ai_binary_id)
            safe_name = re.sub(r"[^A-Za-z0-9_-]", "_", entry.ai_name)[:32] or f"ai_{i}"
            p = MatchParticipant(
                match_id=match.id,
                ai_binary_id=entry.ai_binary_id,
                team_name=safe_name,
                mmr_before=ai.mmr if ai else 1000.0,
            )
            session.add(p)
            participants.append(p)

        await session.commit()
        for p in participants:
            await session.refresh(p)
        await session.refresh(room)

        log.info("Triggering match %d in room %d", match.id, room_id)
        asyncio.create_task(GameInstance(match.id, participants, room).run())
