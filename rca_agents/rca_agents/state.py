"""
Shared state passed between all agents in the RCA LangGraph pipeline.

Flow:
    Runtime Log (+ HMI Screenshot)
        -> Perception Agent      -> observation
        -> Investigation Agent   -> evidence_package
        -> Reasoning Agent       -> root_cause
        -> Report Agent          -> final_report
"""

from __future__ import annotations

from typing import Any, Optional, TypedDict


class Observation(TypedDict, total=False):
    """Normalized output of the Perception Agent."""
    scenario_id: str
    log_file: str
    raw_log: dict
    summary: str                # plain-language description of what the log shows
    flagged_signals: list[dict] # e.g. [{"name": "riskScore", "value": 95, "note": "exceeds brake threshold"}]
    hmi_notes: Optional[str]    # notes from HMI screenshot, if provided


class EvidenceItem(TypedDict, total=False):
    hop: int            # order in which this evidence was gathered (1, 2, 3...)
    source: str          # "neo4j" | "source_code" | "runtime_log"
    tool: str            # tool function name that produced this (e.g. "find_callers")
    query_or_path: str   # cypher query text, or file path
    result: Any


class EvidencePackage(TypedDict, total=False):
    observation: Observation
    call_graph_paths: list[dict]
    data_flow_paths: list[dict]
    relevant_functions: list[dict]  # {name, file, snippet}
    evidence_items: list[EvidenceItem]
    investigation_notes: str


class CallTraceHop(TypedDict, total=False):
    """One hop in the ordered root-cause trace, from entry point to the fault."""
    hop: int
    file: str            # source file, e.g. "ADAS/src/warning.c"
    function: str        # function name, e.g. "EvaluateWarning"
    variables: dict       # relevant variable name -> value at this hop
    expected: str         # what should happen at this hop
    actual: str           # what actually happens / happened at this hop
    is_fault_point: bool  # True if this is where behavior diverges from expected


class RootCause(TypedDict, total=False):
    hypothesis: str
    confidence: str            # "high" | "medium" | "low"
    supporting_evidence: list[str]
    responsible_function: Optional[str]
    responsible_component: Optional[str]  # SWC1..SWC4
    responsible_file: Optional[str]
    call_trace: list[CallTraceHop]  # ordered hop-by-hop trace from entry to fault
    reasoning_trace: str


class RCAState(TypedDict, total=False):
    # inputs
    scenario_file: str          # e.g. "run_003.json"
    hmi_screenshot_path: Optional[str]

    # agent outputs, populated progressively
    observation: Observation
    evidence_package: EvidencePackage
    root_cause: RootCause
    final_report: str

    # bookkeeping
    errors: list[str]
