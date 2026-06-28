from typing import Annotated

from fastapi import APIRouter, Depends, HTTPException, Request
from pydantic import BaseModel
from sqlalchemy.ext.asyncio import AsyncSession

from ..database import get_session
from ..models.ai_binary import AIBinary
from ..models.room import Room
from ..models.user import User
from ..services.matchmaker import QueueEntry
from .auth import get_current_user

router = APIRouter(prefix="/queue", tags=["queue"])


class JoinRequest(BaseModel):
    ai_binary_id: int
    room_id: int


@router.post("/join", status_code=202)
async def join(
    req: JoinRequest,
    request: Request,
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    ai = await session.get(AIBinary, req.ai_binary_id)
    if not ai or ai.user_id != current.id or not ai.is_active:
        raise HTTPException(status_code=404, detail="AI not found")

    room = await session.get(Room, req.room_id)
    if not room or room.status != "open":
        raise HTTPException(status_code=400, detail="Room not open")

    matchmaker = request.app.state.matchmaker
    entry = QueueEntry(ai_binary_id=ai.id, user_id=current.id, ai_name=ai.name, room_id=req.room_id)
    result = await matchmaker.enqueue(entry, session)
    if "error" in result:
        raise HTTPException(status_code=400, detail=result["error"])
    return result


@router.delete("/leave", status_code=200)
async def leave(
    request: Request,
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    from sqlmodel import select
    ais = (
        await session.execute(
            select(AIBinary).where(AIBinary.user_id == current.id, AIBinary.is_active == True)
        )
    ).scalars().all()
    matchmaker = request.app.state.matchmaker
    removed = any([await matchmaker.dequeue(ai.id) for ai in ais])
    return {"removed": removed}
