import asyncio
import stat
import uuid
from pathlib import Path
from typing import Annotated

from fastapi import APIRouter, Depends, HTTPException, UploadFile, Form
from sqlalchemy.ext.asyncio import AsyncSession
from sqlmodel import select

from ..config import settings
from ..database import get_session
from ..models.gui_binary import GuiBinary
from ..models.match import Match
from ..models.user import User
from .auth import get_current_user

router = APIRouter(prefix="/gui", tags=["gui"])


@router.post("/upload", status_code=201)
async def upload(
    file: UploadFile,
    name: Annotated[str, Form()],
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    dest_dir = Path(settings.uploads_dir) / "gui" / str(current.id) / str(uuid.uuid4())
    dest_dir.mkdir(parents=True, exist_ok=True)
    dest = dest_dir / "binary"
    content = await file.read()
    dest.write_bytes(content)
    dest.chmod(dest.stat().st_mode | stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH)

    gui = GuiBinary(user_id=current.id, name=name, file_path=str(dest))
    session.add(gui)
    await session.commit()
    await session.refresh(gui)
    return {"id": gui.id, "name": gui.name}


@router.get("/list")
async def list_guis(
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    results = (await session.execute(
        select(GuiBinary).where(GuiBinary.user_id == current.id, GuiBinary.is_active == True)
    )).scalars().all()
    return [{"id": g.id, "name": g.name, "uploaded_at": g.uploaded_at} for g in results]


@router.delete("/{gui_id}", status_code=204)
async def delete_gui(
    gui_id: int,
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    gui = await session.get(GuiBinary, gui_id)
    if not gui or gui.user_id != current.id:
        raise HTTPException(status_code=404, detail="Not found")
    gui.is_active = False
    session.add(gui)
    await session.commit()


@router.post("/launch/{match_id}", status_code=202)
async def launch_gui(
    match_id: int,
    gui_id: int,
    current: Annotated[User, Depends(get_current_user)],
    session: Annotated[AsyncSession, Depends(get_session)],
):
    match = await session.get(Match, match_id)
    if not match or match.status != "running" or not match.server_port:
        raise HTTPException(status_code=404, detail="Match not running")

    gui = await session.get(GuiBinary, gui_id)
    if not gui or gui.user_id != current.id or not gui.is_active:
        raise HTTPException(status_code=404, detail="GUI binary not found")

    cmd = [gui.file_path, "-p", str(match.server_port), "-h", "localhost"]
    await asyncio.create_subprocess_exec(
        *cmd,
        stdout=asyncio.subprocess.DEVNULL,
        stderr=asyncio.subprocess.DEVNULL,
    )
    return {"launched": True, "port": match.server_port, "gui": gui.name}
