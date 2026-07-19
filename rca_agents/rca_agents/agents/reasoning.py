"""
Reasoning Agent
-----------------
Responsibilities (from project spec):
  - Combine runtime evidence
  - Combine graph evidence
  - Infer software root cause

Input:  state.evidence_package
Output: state.root_cause

This agent is also responsible for producing an ordered, hop-by-hop
CALL TRACE (file -> function -> variables -> expected vs actual) so the
Report Agent can show exactly where in the code execution the behavior
diverges from what was expected, not just a prose conclusion.
"""

from __future__ import annotations

import json

from langchain_openai import ChatOpenAI

from rca_agents.config import OPENAI_MODEL
from rca_agents.state import RCAState, RootCause

SYSTEM_PROMPT = """You are the Reasoning Agent in an ADAS Root Cause Analysis pipeline.

You are given:
  1. An OBSERVATION (normalized runtime log summary, flagged signals, and the raw
     runtime log itself -- which includes sensorData, febData, hmiData, and config
     values such as FEATURE_ENABLE, WARNING_RISK, BRAKE_RISK).
  2. An EVIDENCE PACKAGE (call graph traces, data flow traces, source code snippets,
     and investigation notes gathered by the Investigation Agent via numbered "hops"
     -- each evidence item has a "hop" number, the "tool" used, and the "result").

Your job has two parts:

PART 1 -- Infer the root cause:
  - Was a value computed correctly but the DECISION logic (e.g. threshold check,
    feature flag) suppressed the expected action?
  - Was a value never computed/propagated at all (missing call, broken data flow)?
  - Was the correct decision made but the correct actuation/HMI function never
    invoked, or invoked but implemented as a no-op?

PART 2 -- Build an ordered CALL TRACE from the program's actual execution path
  (main -> SWC1_Run -> SWC2_Run -> SWC3_Run -> SWC4_Run and whatever functions
  those call) to the exact point where behavior diverges from expected. Use the
  file paths and function names found in the evidence (source snippets, call
  graph results) -- do not invent file paths that were not in the evidence. For
  each hop give: the file, the function, the relevant variable(s) and their
  value(s) at that point (from the raw log / evidence), what SHOULD happen at
  that hop given the code logic, what ACTUALLY happens, and whether this hop is
  the fault point (is_fault_point: true for exactly one hop -- the earliest point
  where actual diverges from expected; false for every other hop, which should
  show actual == expected, confirming the rest of the pipeline is not at fault).

Respond ONLY with valid JSON matching this shape:
{
  "hypothesis": "one to three sentence root cause explanation",
  "confidence": "high" | "medium" | "low",
  "supporting_evidence": ["short evidence point 1", "short evidence point 2", ...],
  "responsible_function": "FunctionName or null",
  "responsible_component": "SWC1" | "SWC2" | "SWC3" | "SWC4" | null,
  "responsible_file": "path/to/file.c or null",
  "call_trace": [
    {
      "hop": 1,
      "file": "...",
      "function": "...",
      "variables": {"varName": value, ...},
      "expected": "what should happen here",
      "actual": "what actually happens here",
      "is_fault_point": false
    }
  ],
  "reasoning_trace": "short paragraph walking through how you got from evidence to hypothesis"
}
"""


def run_reasoning_agent(state: RCAState) -> RCAState:
    evidence_package = state["evidence_package"]
    observation = evidence_package.get("observation", {})

    llm = ChatOpenAI(model=OPENAI_MODEL, temperature=0)
    messages = [
        {"role": "system", "content": SYSTEM_PROMPT},
        {
            "role": "user",
            "content": (
                f"Observation (includes raw_log with sensorData/febData/hmiData/config):\n"
                f"{json.dumps(observation, indent=2, default=str)}\n\n"
                f"Investigation notes:\n{evidence_package.get('investigation_notes', '')}\n\n"
                f"Evidence items (numbered hops from Investigation Agent's tool calls):\n"
                f"{json.dumps(evidence_package.get('evidence_items', []), indent=2, default=str)}"
            ),
        },
    ]

    response = llm.invoke(messages)
    content = response.content.strip()

    if content.startswith("```"):
        content = content.strip("`")
        content = content.split("\n", 1)[1] if "\n" in content else content
        if content.endswith("json"):
            content = content[: -len("json")]

    try:
        parsed = json.loads(content)
    except json.JSONDecodeError:
        parsed = {
            "hypothesis": content,
            "confidence": "low",
            "supporting_evidence": [],
            "responsible_function": None,
            "responsible_component": None,
            "responsible_file": None,
            "call_trace": [],
            "reasoning_trace": "Model did not return valid JSON; raw content stored in hypothesis.",
        }

    root_cause: RootCause = {
        "hypothesis": parsed.get("hypothesis", ""),
        "confidence": parsed.get("confidence", "low"),
        "supporting_evidence": parsed.get("supporting_evidence", []),
        "responsible_function": parsed.get("responsible_function"),
        "responsible_component": parsed.get("responsible_component"),
        "responsible_file": parsed.get("responsible_file"),
        "call_trace": parsed.get("call_trace", []),
        "reasoning_trace": parsed.get("reasoning_trace", ""),
    }

    state["root_cause"] = root_cause
    return state
