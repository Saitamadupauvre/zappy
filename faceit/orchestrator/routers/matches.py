from typing import Annotated

from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession
from sqlmodel import select

from ..database import get_session
from ..models.match import Match, MatchParticipant
from ..models.ai_binary import AIBinary
from ..models.user import User
from .auth import get_current_user

router = APIRouter(prefix="/matches", tags=["matches"])


async def _enrich(match: Match, session: AsyncSession) -> dict:
    participants = (
        await session.execute(select(MatchParticipant).where(MatchParticipant.match_id == match.id))
    ).scalars().all()
    parts = []
    for p in participants:
        ai = await session.get(AIBinary, p.ai_binary_id)
        parts.append({
            "ai_binary_id": p.ai_binary_id,
            "ai_name": ai.name if ai else "?",
            "team_name": p.team_name,
            "mmr_before": p.mmr_before,
            "mmr_after": p.mmr_after,
        })
    return {
        "id": match.id,
        "status": match.status,
        "server_port": match.server_port,
        "winner_team": match.winner_team,
        "started_at": match.started_at,
        "finished_at": match.finished_at,
        "participants": parts,
    }


@router.get("")
async def list_matches(
    session: Annotated[AsyncSession, Depends(get_session)],
    _: Annotated[User, Depends(get_current_user)],
    offset: int = 0,
    limit: int = 20,
):
    matches = (
        await session.execute(select(Match).order_by(Match.id.desc()).offset(offset).limit(limit))
    ).scalars().all()
    return [await _enrich(m, session) for m in matches]


@router.get("/live")
async def live_matches(
    session: Annotated[AsyncSession, Depends(get_session)],
):
    matches = (
        await session.execute(select(Match).where(Match.status == "running"))
    ).scalars().all()
    return [{"id": m.id, "port": m.server_port, "started_at": m.started_at} for m in matches]


@router.get("/{match_id}")
async def get_match(
    match_id: int,
    session: Annotated[AsyncSession, Depends(get_session)],
):
    match = await session.get(Match, match_id)
    if not match:
        from fastapi import HTTPException
        raise HTTPException(status_code=404, detail="Match not found")
    return await _enrich(match, session)
