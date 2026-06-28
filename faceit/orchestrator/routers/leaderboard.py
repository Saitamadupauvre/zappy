from typing import Annotated

from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession
from sqlmodel import select

from ..database import get_session
from ..models.ai_binary import AIBinary
from ..models.user import User

router = APIRouter(prefix="/leaderboard", tags=["leaderboard"])


@router.get("")
async def leaderboard(session: Annotated[AsyncSession, Depends(get_session)]):
    ais = (
        await session.execute(
            select(AIBinary).where(AIBinary.is_active == True).order_by(AIBinary.mmr.desc()).limit(50)
        )
    ).scalars().all()
    results = []
    for ai in ais:
        user = await session.get(User, ai.user_id)
        results.append({
            "ai_id": ai.id,
            "name": ai.name,
            "username": user.username if user else "?",
            "mmr": round(ai.mmr, 1),
            "matches_played": ai.matches_played,
        })
    return results
