import os
import stat
import uuid
from pathlib import Path
from typing import Annotated

from fastapi import APIRouter, Depends, HTTPException, UploadFile, Form
from sqlalchemy.ext.asyncio import AsyncSession
from sqlmodel import select

from ..config import settings
from ..database import get_session
from ..models.ai_binary import AIBinary
from ..models.user import User
from .auth import get_current_user

router = APIRouter(prefix="/ai", tags=["ai"])


@router.post("/upload", status_code=201)
async def upload(
    file: UploadFile,
    name: Annotated[str, Form()],
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    dest_dir = Path(settings.uploads_dir) / str(current.id) / str(uuid.uuid4())
    dest_dir.mkdir(parents=True, exist_ok=True)
    dest = dest_dir / "binary"
    content = await file.read()
    dest.write_bytes(content)
    dest.chmod(dest.stat().st_mode | stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH)

    ai = AIBinary(user_id=current.id, name=name, file_path=str(dest))
    session.add(ai)
    await session.commit()
    await session.refresh(ai)
    return {"id": ai.id, "name": ai.name, "mmr": ai.mmr}


@router.get("/list")
async def list_ai(
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    results = (await session.execute(
        select(AIBinary).where(AIBinary.user_id == current.id, AIBinary.is_active == True)
    )).scalars().all()
    return [
        {"id": a.id, "name": a.name, "mmr": a.mmr, "matches_played": a.matches_played}
        for a in results
    ]


@router.delete("/{ai_id}", status_code=204)
async def delete_ai(
    ai_id: int,
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    ai = await session.get(AIBinary, ai_id)
    if not ai or ai.user_id != current.id:
        raise HTTPException(status_code=404, detail="Not found")
    ai.is_active = False
    session.add(ai)
    await session.commit()
