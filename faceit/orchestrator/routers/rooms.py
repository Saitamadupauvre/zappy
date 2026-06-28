from typing import Annotated

from fastapi import APIRouter, Depends, HTTPException, Request
from pydantic import BaseModel
from sqlalchemy.ext.asyncio import AsyncSession
from sqlmodel import select

from ..database import get_session
from ..models.ai_binary import AIBinary
from ..models.room import Room
from ..models.server_binary import ServerBinary
from ..models.user import User
from .auth import get_current_user

router = APIRouter(prefix="/rooms", tags=["rooms"])


class CreateRoomRequest(BaseModel):
    server_binary_id: int
    max_players: int = 4


@router.post("", status_code=201)
async def create_room(
    req: CreateRoomRequest,
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    srv = await session.get(ServerBinary, req.server_binary_id)
    if not srv or srv.user_id != current.id or not srv.is_active:
        raise HTTPException(status_code=404, detail="Server binary not found")

    if req.max_players not in (2, 4):
        raise HTTPException(status_code=400, detail="max_players must be 2 or 4")
    room = Room(host_user_id=current.id, server_binary_id=srv.id, max_players=req.max_players)
    session.add(room)
    await session.commit()
    await session.refresh(room)
    return {"id": room.id, "status": room.status, "created_at": room.created_at}


@router.get("")
async def list_rooms(
    request: Request,
    session: Annotated[AsyncSession, Depends(get_session)],
    current: Annotated[User, Depends(get_current_user)],
):
    rooms = (await session.execute(
        select(Room).where(Room.status == "open").order_by(Room.created_at)
    )).scalars().all()
    matchmaker = request.app.state.matchmaker

    result = []
    for r in rooms:
        host = await session.get(User, r.host_user_id)
        entries = matchmaker.room_entries(r.id)
        players = []
        for e in entries:
            ai = await session.get(AIBinary, e.ai_binary_id)
            owner = await session.get(User, e.user_id)
            players.append({
                "ai_name": e.ai_name,
                "username": owner.username if owner else "?",
                "mmr": ai.mmr if ai else 1000.0,
            })
        result.append({
            "id": r.id,
            "host_username": host.username if host else "?",
            "status": r.status,
            "created_at": r.created_at,
            "players": players,
            "slots": len(players),
            "max_players": r.max_players,
        })
    return result


@router.delete("/{room_id}", status_code=204)
async def close_room(
    room_id: int,
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    room = await session.get(Room, room_id)
    if not room or room.host_user_id != current.id:
        raise HTTPException(status_code=404, detail="Room not found")
    if room.status != "open":
        raise HTTPException(status_code=409, detail="Room not open")
    room.status = "closed"
    session.add(room)
    await session.commit()
