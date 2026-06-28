from sqlalchemy.ext.asyncio import AsyncSession
from sqlmodel import select

from ..models.ai_binary import AIBinary
from ..models.match import Match, MatchParticipant

K = 32.0
MMR_FLOOR = 100.0


def _expected(rating_a: float, rating_b: float) -> float:
    return 1.0 / (1.0 + 10.0 ** ((rating_b - rating_a) / 400.0))


async def update_match(match_id: int, winner_team: str, session: AsyncSession) -> None:
    participants = (
        await session.execute(select(MatchParticipant).where(MatchParticipant.match_id == match_id))
    ).scalars().all()

    winner = next((p for p in participants if p.team_name == winner_team), None)
    losers = [p for p in participants if p.team_name != winner_team]

    if not winner:
        return

    deltas: dict[int, float] = {p.id: 0.0 for p in participants}

    for loser in losers:
        e_win = _expected(winner.mmr_before, loser.mmr_before)
        deltas[winner.id] += K * (1.0 - e_win)
        deltas[loser.id] += K * (0.0 - (1.0 - e_win))

    for p in participants:
        new_mmr = max(MMR_FLOOR, p.mmr_before + deltas[p.id])
        p.mmr_after = new_mmr
        session.add(p)
        ai = await session.get(AIBinary, p.ai_binary_id)
        if ai:
            ai.mmr = new_mmr
            ai.matches_played += 1
            session.add(ai)

    match = await session.get(Match, match_id)
    if match:
        from datetime import datetime
        match.winner_team = winner_team
        match.status = "finished"
        match.finished_at = datetime.utcnow()
        session.add(match)

    await session.commit()
