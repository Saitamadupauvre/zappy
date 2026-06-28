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
from ..models.server_binary import ServerBinary
from ..models.user import User
from .auth import get_current_user

router = APIRouter(prefix="/server", tags=["server"])


@router.post("/upload", status_code=201)
async def upload(
    file: UploadFile,
    name: Annotated[str, Form()],
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    dest_dir = Path(settings.uploads_dir) / "servers" / str(current.id) / str(uuid.uuid4())
    dest_dir.mkdir(parents=True, exist_ok=True)
    dest = dest_dir / "binary"
    content = await file.read()
    dest.write_bytes(content)
    dest.chmod(dest.stat().st_mode | stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH)

    srv = ServerBinary(user_id=current.id, name=name, file_path=str(dest))
    session.add(srv)
    await session.commit()
    await session.refresh(srv)
    return {"id": srv.id, "name": srv.name}


@router.get("/list")
async def list_servers(
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    results = (await session.execute(
        select(ServerBinary).where(
            ServerBinary.user_id == current.id,
            ServerBinary.is_active == True,
        )
    )).scalars().all()
    return [{"id": s.id, "name": s.name, "uploaded_at": s.uploaded_at} for s in results]


@router.delete("/{server_id}", status_code=204)
async def delete_server(
    server_id: int,
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    srv = await session.get(ServerBinary, server_id)
    if not srv or srv.user_id != current.id:
        raise HTTPException(status_code=404, detail="Not found")
    srv.is_active = False
    session.add(srv)
    await session.commit()
