"""File-based lookups: runtime logs and source code snippets."""

from __future__ import annotations

import base64
import json
from pathlib import Path

from rca_agents.config import LOGS_DIR, SOURCE_DIR


def load_runtime_log(log_filename: str) -> dict:
    """Load a runtime execution log, e.g. 'run_003.json'."""
    path = Path(LOGS_DIR) / log_filename
    if not path.exists():
        raise FileNotFoundError(f"Runtime log not found: {path}")
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


import re


def find_function_in_source(function_name: str, search_root: Path = SOURCE_DIR) -> dict | None:
    """
    Best-effort locate a function DEFINITION (not a call site) in the C
    source tree and return its file path plus a surrounding snippet.

    This is intentionally simple (regex, not full parsing) since the
    CPG/Joern side already owns precise structural analysis; this is just
    for pulling a human-readable snippet into the evidence package.

    Matches lines like:
        int EvaluateBrake(int riskScore)
        void UpdateBrakeLamp(int brakeRequest)
        static void StoreFEBOutput(int risk, ...)
    i.e. a line that is itself the function signature (return type +
    name + open paren), as opposed to a call embedded in another
    statement (`brake = EvaluateBrake(risk);`).
    """
    search_root = Path(search_root)
    if not search_root.exists():
        return None

    # Definition: start-of-line (optional whitespace), optional 'static',
    # a return type token, the function name, then '('. Deliberately does
    # NOT require the line to end in '{' since some codebases put the
    # brace on the next line.
    def_pattern = re.compile(
        rf"^\s*(static\s+)?[A-Za-z_][\w\s\*]*?\b{re.escape(function_name)}\s*\(",
        re.MULTILINE,
    )

    candidates: list[tuple[Path, str, int]] = []
    for c_file in search_root.rglob("*.c"):
        try:
            text = c_file.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        for m in def_pattern.finditer(text):
            candidates.append((c_file, text, m.start()))

    if not candidates:
        # Fall back to a plain substring match (e.g. header-only declarations,
        # or if the definition style doesn't match the regex) rather than
        # returning nothing.
        marker = f"{function_name}("
        for c_file in search_root.rglob("*.c"):
            try:
                text = c_file.read_text(encoding="utf-8", errors="ignore")
            except OSError:
                continue
            idx = text.find(marker)
            if idx != -1:
                candidates.append((c_file, text, idx))
                break

    if not candidates:
        return None

    c_file, text, idx = candidates[0]
    lines = text.splitlines()
    line_no = text[:idx].count("\n")
    start = max(0, line_no - 2)
    end = min(len(lines), line_no + 25)
    snippet = "\n".join(lines[start:end])

    return {
        "function": function_name,
        "file": str(c_file.relative_to(search_root.parent) if search_root.parent in c_file.parents else c_file),
        "line": line_no + 1,
        "snippet": snippet,
    }


def load_hmi_screenshot_note(path: str | None) -> str | None:
    """
    Validates an HMI screenshot path exists. Kept for backwards-compat /
    logging; actual image bytes are loaded separately by
    load_hmi_screenshot_base64() for the vision call.
    """
    if not path:
        return None
    p = Path(path)
    if not p.exists():
        return f"[screenshot referenced but not found on disk: {path}]"
    return f"[screenshot available at {path}]"


_MEDIA_TYPES = {
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".webp": "image/webp",
    ".gif": "image/gif",
}


def load_hmi_screenshot_base64(path: str | None) -> dict | None:
    """
    Load an HMI screenshot as base64 for a vision-capable chat model.

    Returns a dict {"media_type": ..., "data": ...} or None if no path was
    given or the file doesn't exist / isn't a supported image type.
    """
    if not path:
        return None
    p = Path(path)
    if not p.exists():
        return None

    media_type = _MEDIA_TYPES.get(p.suffix.lower())
    if media_type is None:
        return None

    with open(p, "rb") as f:
        data = base64.b64encode(f.read()).decode("utf-8")

    return {"media_type": media_type, "data": data}
