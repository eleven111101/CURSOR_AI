"""
Report Agent
-------------
Responsibilities (from project spec):
  - Generate structured RCA report
  - Explain observations
  - Present supporting evidence
  - Recommend corrective actions

Additionally (per requirements):
  - Name every file/function that played a role in the investigation
  - Show a hop-by-hop code trace with variable values, expected vs actual
  - Report how many hops/tool calls the Investigation Agent used to reach
    the root cause

Input:  full state (observation, evidence_package, root_cause)
Output: state.final_report (markdown string)
"""

from __future__ import annotations

import json

from langchain_openai import ChatOpenAI

from rca_agents.config import OPENAI_MODEL
from rca_agents.state import RCAState

SYSTEM_PROMPT = """You are the Report Agent in an ADAS Root Cause Analysis pipeline.

You are given the full analysis trail: the original observation (including the raw
runtime log), every evidence item the Investigation Agent gathered (each tagged with
a hop number and the tool/query used), and the Reasoning Agent's root cause conclusion
including an ordered call_trace.

Produce a clear, structured RCA report in Markdown intended for an embedded software
engineer to read. The report MUST include these sections, in this order, and MUST be
concrete -- name real file paths and function names from the evidence, never generic
placeholders:

# RCA Report: <scenario id>

## Summary
2-4 sentences: what was observed, what the root cause is, confidence level.

## Files & Functions Involved
A table of every file and function that appears anywhere in the evidence or call
trace, with a one-line note on its role (e.g. "computes riskScore", "gates warning
lamp on FEATURE_ENABLE", "no-op, does not write brakeLamp"). Pull this from the
evidence -- do not invent files/functions that were not in the evidence package.

## Code Trace (hop by hop)
Render the root cause agent's call_trace as a table with columns:
Hop | File | Function | Variables at this point | Expected | Actual | Fault point?
Mark the fault-point row clearly (e.g. bold, or an explicit "<-- FAULT" marker in the
last column). This is the most important section -- it must let an engineer read
top to bottom and see exactly where behavior diverges from expected.

## Root Cause
The specific, concrete explanation (not generic). State the responsible file and
function explicitly.

## Expected vs Actual (Case Comparison)
If the observation/evidence includes a comparison between an expected/working case
and the actual/issue case (e.g. two scenarios with identical inputs but different
outcomes), render a side-by-side table of the differing fields only (skip fields
that are identical in both). If only one case is available, instead show a table of
"Expected behavior per source code logic" vs "Actual behavior observed in this run".

## Supporting Evidence
Bullet list citing specific evidence items (reference hop numbers from the
Investigation Agent, e.g. "Hop 3: get_source_snippet(UpdateBrakeLamp) confirmed the
function body is a no-op").

## Affected Component(s)
Which SWC(s) / files are affected.

## Investigation Effort
State plainly: how many tool calls / hops the Investigation Agent used to reach this
conclusion, and which tools were used (e.g. "5 hops: 2 Neo4j call-graph queries, 1
variable-reader query, 2 source snippet lookups").

## Recommended Corrective Actions
Concrete, specific to the responsible file/function -- not generic advice.

Keep it technical and precise. Do not invent evidence, file paths, or function names
that are not present in the observation, evidence items, or call_trace you were
given. If confidence is "low" or "medium", say so explicitly and note what additional
evidence would increase confidence.
"""


def _summarize_investigation_effort(evidence_items: list[dict]) -> str:
    if not evidence_items:
        return "No investigation evidence was gathered."
    tool_counts: dict[str, int] = {}
    for item in evidence_items:
        tool = item.get("tool", item.get("query_or_path", "unknown").split("(")[0])
        tool_counts[tool] = tool_counts.get(tool, 0) + 1
    parts = ", ".join(f"{count}x {tool}" for tool, count in tool_counts.items())
    return f"{len(evidence_items)} hop(s) total: {parts}"


def run_report_agent(state: RCAState) -> RCAState:
    observation = state.get("observation", {})
    evidence_package = state.get("evidence_package", {})
    root_cause = state.get("root_cause", {})
    evidence_items = evidence_package.get("evidence_items", [])

    investigation_effort = _summarize_investigation_effort(evidence_items)

    llm = ChatOpenAI(model=OPENAI_MODEL, temperature=0)
    messages = [
        {"role": "system", "content": SYSTEM_PROMPT},
        {
            "role": "user",
            "content": (
                f"Scenario: {observation.get('scenario_id', state.get('scenario_file', 'unknown'))}\n\n"
                f"Observation (includes raw_log with sensorData/febData/hmiData/config):\n"
                f"{json.dumps(observation, indent=2, default=str)}\n\n"
                f"Investigation notes:\n{evidence_package.get('investigation_notes', '')}\n\n"
                f"Evidence items (hop-numbered, from Investigation Agent tool calls):\n"
                f"{json.dumps(evidence_items, indent=2, default=str)}\n\n"
                f"Investigation effort summary (use this verbatim in the "
                f"'Investigation Effort' section): {investigation_effort}\n\n"
                f"Root cause analysis (includes ordered call_trace):\n"
                f"{json.dumps(root_cause, indent=2, default=str)}"
            ),
        },
    ]

    response = llm.invoke(messages)
    state["final_report"] = response.content
    return state
