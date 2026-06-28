from datetime import datetime
from typing import Optional
from sqlmodel import SQLModel, Field


class ServerBinary(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    user_id: int = Field(foreign_key="user.id", index=True)
    name: str
    file_path: str
    uploaded_at: datetime = Field(default_factory=datetime.utcnow)
    is_active: bool = Field(default=True)
