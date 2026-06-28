from datetime import datetime
from typing import Optional
from sqlmodel import SQLModel, Field


class Room(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    host_user_id: int = Field(foreign_key="user.id", index=True)
    server_binary_id: int = Field(foreign_key="serverbinary.id")
    server_port: Optional[int] = None
    status: str = Field(default="open")  # open | running | closed
    match_id: Optional[int] = Field(default=None, foreign_key="match.id")
    max_players: int = Field(default=4)
    created_at: datetime = Field(default_factory=datetime.utcnow)
