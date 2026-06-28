from contextlib import asynccontextmanager
from pathlib import Path

from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles

from .database import init_db
from .routers import auth, ai, queue, matches, leaderboard, server, rooms, gui
from .services.matchmaker import MatchmakerService


@asynccontextmanager
async def lifespan(app: FastAPI):
    await init_db()
    app.state.matchmaker = MatchmakerService()
    yield


app = FastAPI(title="Zappy FACEIT", lifespan=lifespan)

app.include_router(auth.router, prefix="/api")
app.include_router(ai.router, prefix="/api")
app.include_router(queue.router, prefix="/api")
app.include_router(matches.router, prefix="/api")
app.include_router(leaderboard.router, prefix="/api")
app.include_router(server.router, prefix="/api")
app.include_router(rooms.router, prefix="/api")
app.include_router(gui.router, prefix="/api")

portal_dir = Path(__file__).parent.parent / "portal"
if portal_dir.exists():
    app.mount("/", StaticFiles(directory=str(portal_dir), html=True), name="portal")
