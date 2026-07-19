"""Central configuration: env vars, paths, and shared clients."""

from __future__ import annotations

import os
from pathlib import Path

from dotenv import load_dotenv

load_dotenv()

# --- OpenAI ---
OPENAI_API_KEY = os.environ.get("OPENAI_API_KEY", "")
OPENAI_MODEL = os.environ.get("OPENAI_MODEL", "gpt-4o")

# --- Neo4j ---
NEO4J_URI = os.environ.get("NEO4J_URI", "bolt://localhost:7687")
NEO4J_USER = os.environ.get("NEO4J_USER", "neo4j")
NEO4J_PASSWORD = os.environ.get("NEO4J_PASSWORD", "password")

# --- Project paths ---
PROJECT_ROOT = Path(os.environ.get("ADAS_PROJECT_ROOT", ".")).resolve()
OUTPUT_DIR = PROJECT_ROOT / os.environ.get("OUTPUT_DIR", "output")
LOGS_DIR = PROJECT_ROOT / os.environ.get("LOGS_DIR", "execution/logs")
SOURCE_DIR = PROJECT_ROOT / os.environ.get("SOURCE_DIR", "ADAS")


def require_openai_key() -> None:
    if not OPENAI_API_KEY:
        raise RuntimeError(
            "OPENAI_API_KEY is not set. Copy .env.example to .env and fill it in."
        )
