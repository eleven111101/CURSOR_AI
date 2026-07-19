"""
Investigation Agent
---------------------
Responsibilities (from project spec):
  - Query Neo4j
  - Traverse call graph
  - Traverse data flow
  - Retrieve source code
  - Build evidence package

Input:  state.observation
Output: state.evidence_package

Implementation note: the LLM is given tools (LangChain @tool) wrapping the
Neo4j client and source lookup, and is allowed to call them iteratively
(bounded by MAX_TOOL_CALLS) before summarizing what it found.
"""

from __future__ import annotations

import json

from langchain_core.messages import AIMessage, HumanMessage, SystemMessage, ToolMessage
from langchain_core.tools import tool
from langchain_openai import ChatOpenAI

from rca_agents.config import OPENAI_MODEL
from rca_agents.state import EvidenceItem, EvidencePackage, RCAState
from rca_agents.tools.file_tools import find_function_in_source
from rca_agents.tools.neo4j_client import Neo4jClient

MAX_TOOL_CALLS = 8

# If a Neo4j call fails with one of these signatures, treat it as a
# connection/auth problem (not "no data found") and stop retrying Neo4j
# tools for the rest of this run -- retrying a bad password 8 times wastes
# the entire tool-call budget and produces zero usable evidence.
_NEO4J_AUTH_ERROR_MARKERS = (
    "unauthorized",
    "authentication",
    "auth failure",
    "servicenotavailable",
    "connection refused",
    "could not connect",
    "unable to retrieve routing",
    "failed to establish connection",
)

_neo4j = Neo4jClient()


def _is_neo4j_auth_error(error_text: str) -> bool:
    lowered = error_text.lower()
    return any(marker in lowered for marker in _NEO4J_AUTH_ERROR_MARKERS)


@tool
def find_callers(function_name: str) -> str:
    """Find all functions that call the given function name in the call graph."""
    return json.dumps(_neo4j.find_callers(function_name))


@tool
def find_callees(function_name: str) -> str:
    """Find all functions called BY the given function name."""
    return json.dumps(_neo4j.find_callees(function_name))


@tool
def variable_readers(variable_name: str) -> str:
    """Find all functions that read/reference a given struct field or scalar
    variable by its bare name (e.g. 'riskScore', 'warningRequest',
    'FEATURE_ENABLE' -- NOT 'febData.riskScore', just 'riskScore'). Backed
    by Identifier nodes and USES_IDENTIFIER/READS edges. Also returns the
    actual reading expressions (e.g. 'riskScore >= WARNING_RISK') and any
    controlling if-statement, which is useful for spotting threshold/gating
    logic around the variable."""
    return json.dumps(_neo4j.variable_readers(variable_name))


@tool
def variable_writers(variable_name: str) -> str:
    """Find functions that write/assign a given struct field or scalar
    variable by its bare name (e.g. 'riskScore', not 'febData.riskScore').
    Best-effort: looks for assignment-style expressions involving the
    identifier. If this returns no assignment expression but does return
    functions, those functions still reference the variable -- read the
    'assignmentExpression' field per row to see if a write was found."""
    return json.dumps(_neo4j.variable_writers(variable_name))


@tool
def call_chain_between(from_function: str, to_function: str) -> str:
    """Find the shortest call-graph path between two functions, if one exists."""
    return json.dumps(_neo4j.call_chain_between(from_function, to_function))


@tool
def execution_path_from_main() -> str:
    """Get the full reachable call path starting from main()."""
    return json.dumps(_neo4j.execution_path_from_main())


@tool
def get_source_snippet(function_name: str) -> str:
    """Retrieve the source code snippet for a given function name from the ADAS source tree."""
    result = find_function_in_source(function_name)
    if result is None:
        return json.dumps({"error": f"function '{function_name}' not found in source tree"})
    return json.dumps(result)


TOOLS = [
    find_callers,
    find_callees,
    variable_readers,
    variable_writers,
    call_chain_between,
    execution_path_from_main,
    get_source_snippet,
]
TOOLS_BY_NAME = {t.name: t for t in TOOLS}

SYSTEM_PROMPT = """You are the Investigation Agent in an ADAS Root Cause Analysis pipeline.

You receive a normalized OBSERVATION describing a runtime scenario (this may be a \
failure with flagged signals, OR a normal/expected-behavior case with no anomalies --
either way, your job is to gather EVIDENCE, not to conclude the root cause yet).

The software has 4 components:
  SWC1 (sensor processing) -> SWC2 (FEB logic: risk score, warning/brake decisions)
  -> SWC3 (HMI) -> SWC4 (output/actuation/logging)

Use the available tools to:
  1. Trace which functions read/write the flagged variables (e.g. riskScore, \
     warningRequest, brakeRequest, brakeApplied, featureEnable). IMPORTANT: pass \
     BARE variable/field names to variable_readers/variable_writers -- use \
     "riskScore", NOT "febData.riskScore" or "febData.riskScore" with the struct \
     prefix. The graph indexes each field by its own name, not the struct-qualified form.
  2. Trace the call graph around those functions (who calls them, what do they call).
  3. REQUIRED, even when nothing looks wrong: call get_source_snippet at least once \
     for the primary decision function most relevant to the observation (e.g. \
     EvaluateWarning or EvaluateBrake for a risk/warning scenario), to confirm the \
     real file path and exact source text. Do not skip this just because the case \
     looks like normal/expected behavior with no fault -- file-level confirmation is \
     what lets the Reasoning Agent report high confidence instead of medium, even for \
     a "nothing is wrong here" conclusion.

Be efficient: you have at most """ + str(MAX_TOOL_CALLS) + """ tool calls. Prioritize \
the flagged signals from the observation, but always spend at least one call on \
get_source_snippet per point 3 above. Once you have enough evidence to hand off to \
the Reasoning Agent, STOP calling tools and respond with a final plain-text summary of \
what you found (which functions/paths are implicated and why), not JSON.
"""


def run_investigation_agent(state: RCAState) -> RCAState:
    observation = state["observation"]

    llm = ChatOpenAI(model=OPENAI_MODEL, temperature=0).bind_tools(TOOLS)

    messages = [
        SystemMessage(content=SYSTEM_PROMPT),
        HumanMessage(
            content=(
                f"Observation summary: {observation.get('summary')}\n\n"
                f"Flagged signals: {json.dumps(observation.get('flagged_signals', []), indent=2)}"
            )
        ),
    ]

    evidence_items: list[EvidenceItem] = []
    tool_calls_made = 0
    neo4j_disabled = False  # flips True after the first confirmed auth/connection failure
    neo4j_tool_names = {t.name for t in TOOLS if t.name != "get_source_snippet"}

    while tool_calls_made < MAX_TOOL_CALLS:
        response: AIMessage = llm.invoke(messages)
        messages.append(response)

        if not response.tool_calls:
            # Model decided it has enough evidence
            investigation_notes = response.content
            break

        neo4j_just_disabled = False

        for tc in response.tool_calls:
            if tool_calls_made >= MAX_TOOL_CALLS:
                # Budget exhausted mid-batch. Every tool_call in this
                # response still needs a ToolMessage -- OpenAI rejects an
                # assistant message with unanswered tool_call_ids. Answer
                # the remaining ones with a "budget exhausted" placeholder
                # rather than breaking and leaving them unanswered.
                result_str = json.dumps({
                    "error": "tool_call_budget_exhausted",
                    "reason": f"MAX_TOOL_CALLS ({MAX_TOOL_CALLS}) reached before this call could run.",
                })
                messages.append(ToolMessage(content=result_str, tool_call_id=tc["id"]))
                continue

            # If Neo4j already confirmed broken this run, don't spend another
            # hop re-trying it -- short-circuit with a clear error and nudge
            # the model toward source-code tools instead.
            if neo4j_disabled and tc["name"] in neo4j_tool_names:
                result_str = json.dumps({
                    "error": "neo4j_disabled_for_this_run",
                    "reason": (
                        "A prior call to this tool failed with an authentication/connection "
                        "error. Neo4j tools are being skipped for the rest of this run to avoid "
                        "wasting the tool-call budget. Use get_source_snippet instead to gather "
                        "evidence directly from source code."
                    ),
                })
                evidence_items.append({
                    "hop": len(evidence_items) + 1,
                    "source": "neo4j",
                    "tool": tc["name"],
                    "query_or_path": f"{tc['name']}({tc['args']})",
                    "result": result_str,
                })
                messages.append(ToolMessage(content=result_str, tool_call_id=tc["id"]))
                tool_calls_made += 1
                continue

            tool_fn = TOOLS_BY_NAME.get(tc["name"])
            if tool_fn is None:
                result_str = json.dumps({"error": f"unknown tool {tc['name']}"})
            else:
                try:
                    result_str = tool_fn.invoke(tc["args"])
                except Exception as e:  # noqa: BLE001
                    result_str = json.dumps({"error": str(e)})

            # Detect a fresh Neo4j auth/connection failure and disable Neo4j
            # tools for the remainder of this run.
            if tc["name"] in neo4j_tool_names and _is_neo4j_auth_error(result_str):
                if not neo4j_disabled:
                    neo4j_just_disabled = True
                neo4j_disabled = True

            evidence_items.append(
                {
                    "hop": len(evidence_items) + 1,
                    "source": "neo4j" if tc["name"] != "get_source_snippet" else "source_code",
                    "tool": tc["name"],
                    "query_or_path": f"{tc['name']}({tc['args']})",
                    "result": result_str,
                }
            )
            messages.append(ToolMessage(content=result_str, tool_call_id=tc["id"]))
            tool_calls_made += 1

        # IMPORTANT: only append a plain HumanMessage AFTER every tool_call_id
        # from this assistant turn has a matching ToolMessage. OpenAI's API
        # requires all tool responses for a batch of tool_calls to be
        # contiguous with no other message type interleaved -- inserting a
        # nudge message mid-batch causes a 400 error on the next call.
        if neo4j_just_disabled:
            messages.append(
                HumanMessage(
                    content=(
                        "Neo4j appears to be unreachable or misconfigured (authentication/"
                        "connection error). Do not call find_callers, find_callees, "
                        "variable_readers, variable_writers, call_chain_between, or "
                        "execution_path_from_main again this run. Continue the "
                        "investigation using get_source_snippet against the source files "
                        "you already know are relevant (main.c, SWC1_Run..SWC4_Run and the "
                        "functions they call), based on the flagged signals."
                    )
                )
            )
    else:
        # Hit the tool-call budget; force a final summary
        messages.append(
            HumanMessage(
                content="You have reached the tool call budget. Summarize your findings now in plain text."
            )
        )
        final = llm.invoke(messages)
        investigation_notes = final.content

    evidence_package: EvidencePackage = {
        "observation": observation,
        "evidence_items": evidence_items,
        "investigation_notes": investigation_notes,
    }

    state["evidence_package"] = evidence_package
    return state