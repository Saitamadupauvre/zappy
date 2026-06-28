from datetime import datetime
from typing import Optional
from sqlmodel import SQLModel, Field


class AIBinary(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    user_id: int = Field(foreign_key="user.id", index=True)
    name: str
    file_path: str
    mmr: float = Field(default=1000.0)
    matches_played: int = Field(default=0)
    uploaded_at: datetime = Field(default_factory=datetime.utcnow)
    is_active: bool = Field(default=True)
