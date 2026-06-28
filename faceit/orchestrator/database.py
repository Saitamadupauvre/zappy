from sqlmodel import SQLModel
from .models import User, AIBinary, ServerBinary, GuiBinary, Match, MatchParticipant, Room  # noqa: F401 — required for SQLModel metadata
from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession
from sqlalchemy.orm import sessionmaker
from .config import settings

engine = create_async_engine(f"sqlite+aiosqlite:///{settings.db_path}", echo=False)

AsyncSessionLocal = sessionmaker(engine, class_=AsyncSession, expire_on_commit=False)


async def init_db() -> None:
    async with engine.begin() as conn:
        await conn.run_sync(SQLModel.metadata.create_all)


async def get_session():
    async with AsyncSessionLocal() as session:
        yield session
