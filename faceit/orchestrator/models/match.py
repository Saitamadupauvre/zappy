from datetime import datetime
from typing import Optional
from sqlmodel import SQLModel, Field


class Match(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    server_port: Optional[int] = None
    status: str = Field(default="pending")  # pending | running | finished | error
    winner_team: Optional[str] = None
    started_at: Optional[datetime] = None
    finished_at: Optional[datetime] = None
    map_width: int = Field(default=10)
    map_height: int = Field(default=10)
    freq: int = Field(default=100)


class MatchParticipant(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    match_id: int = Field(foreign_key="match.id", index=True)
    ai_binary_id: int = Field(foreign_key="aibinary.id")
    team_name: str
    mmr_before: float
    mmr_after: Optional[float] = None
