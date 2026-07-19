"""
Perception Agent
-----------------
Responsibilities (from project spec):
  - Parse runtime logs
  - Read HMI screenshots (actual image analysis, not just a file-exists note)
  - Produce normalized observations

Input:  state.scenario_file, state.hmi_screenshot_path
Output: state.observation
"""

from __future__ import annotations

import json

from langchain_openai import ChatOpenAI

from rca_agents.config import OPENAI_MODEL
from rca_agents.state import Observation, RCAState
from rca_agents.tools.file_tools import load_hmi_screenshot_base64, load_runtime_log

SYSTEM_PROMPT = """You are the Perception Agent in an embedded-software Root Cause \
Analysis pipeline for an ADAS (Advanced Driver Assistance System).

You are given:
  1. A raw runtime execution log (JSON) from a Forward Emergency Braking system with \
     four stages: SWC1 (sensor processing), SWC2 (FEB logic: risk scoring, warning/brake \
     decisions), SWC3 (HMI: lamps/buzzer/display), SWC4 (output: dashboard, actuator, logger).
  2. Optionally, a photo/screenshot of the HMI (instrument cluster / dashboard) at the \
     time of the incident, showing which lamps, buzzer indicators, or display messages \
     were actually active.

Your job is ONLY to observe and normalize -- do NOT diagnose the root cause yet. \
That is a later agent's job.

If an image is provided, actually look at it and note what it shows: which warning \
lamp(s), brake indicator(s), buzzer icon, or display text/values are visible/lit, and \
whether that visually matches or contradicts what the log signals say should be \
happening. A mismatch between what the log says and what the HMI image shows is itself \
an important flagged item.

Produce:
1. A short plain-language summary of what the log (and image, if given) shows (2-4 sentences).
2. A list of "flagged_signals": any values or visual states that look anomalous, \
   threshold-crossing, or inconsistent with expected system behavior -- including any \
   mismatch between the log data and the HMI image. For each, give name, value, and a \
   short note.

Respond ONLY with valid JSON matching this shape:
{
  "summary": "...",
  "flagged_signals": [{"name": "...", "value": ..., "note": "..."}],
  "hmi_image_observations": "what the image shows, or null if no image was provided"
}
"""


def _build_user_content(scenario_file: str, raw_log: dict, image_b64: dict | None) -> list[dict] | str:
    text_block = {
        "type": "text",
        "text": (
            f"Scenario file: {scenario_file}\n\n"
            f"Raw log JSON:\n{json.dumps(raw_log, indent=2)}\n\n"
            + ("An HMI screenshot is attached below." if image_b64 else "No HMI screenshot was provided.")
        ),
    }

    if image_b64 is None:
        return text_block["text"]

    image_block = {
        "type": "image_url",
        "image_url": {"url": f"data:{image_b64['media_type']};base64,{image_b64['data']}"},
    }
    return [text_block, image_block]


def run_perception_agent(state: RCAState) -> RCAState:
    scenario_file = state["scenario_file"]
    raw_log = load_runtime_log(scenario_file)
    image_b64 = load_hmi_screenshot_base64(state.get("hmi_screenshot_path"))

    llm = ChatOpenAI(model=OPENAI_MODEL, temperature=0)
    messages = [
        {"role": "system", "content": SYSTEM_PROMPT},
        {"role": "user", "content": _build_user_content(scenario_file, raw_log, image_b64)},
    ]

    response = llm.invoke(messages)
    content = response.content.strip()

    # Defensive parsing: strip markdown fences if the model adds them anyway
    if content.startswith("```"):
        content = content.strip("`")
        content = content.split("\n", 1)[1] if "\n" in content else content
        if content.endswith("json"):
            content = content[: -len("json")]

    try:
        parsed = json.loads(content)
    except json.JSONDecodeError:
        parsed = {
            "summary": content,
            "flagged_signals": [],
            "hmi_image_observations": None,
        }

    observation: Observation = {
        "scenario_id": scenario_file.replace(".json", ""),
        "log_file": scenario_file,
        "raw_log": raw_log,
        "summary": parsed.get("summary", ""),
        "flagged_signals": parsed.get("flagged_signals", []),
        "hmi_notes": parsed.get("hmi_image_observations"),
    }

    state["observation"] = observation
    return state
