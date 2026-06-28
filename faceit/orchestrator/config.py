import os
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent.parent

class Settings:
    db_path: str = str(BASE_DIR / "faceit.db")
    uploads_dir: str = str(BASE_DIR / "uploads")
    secret_key: str = os.environ.get("FACEIT_SECRET", "change-me-in-production")
    algorithm: str = "HS256"
    token_expire_minutes: int = 60 * 24
    map_width: int = 10
    map_height: int = 10
    freq: int = 100
    match_timeout: float = 3600.0

settings = Settings()
