"""
LangGraph orchestration for the 4-agent RCA pipeline.

    Perception -> Investigation -> Reasoning -> Report

This is intentionally a linear graph for the POC (matches the fixed pipeline
in the project README). Each node mutates/returns the shared RCAState.
"""

from __future__ import annotations

from langgraph.graph import END, StateGraph

from rca_agents.agents.investigation import run_investigation_agent
from rca_agents.agents.perception import run_perception_agent
from rca_agents.agents.reasoning import run_reasoning_agent
from rca_agents.agents.report import run_report_agent
from rca_agents.state import RCAState


def build_rca_graph():
    graph = StateGraph(RCAState)

    graph.add_node("perception", run_perception_agent)
    graph.add_node("investigation", run_investigation_agent)
    graph.add_node("reasoning", run_reasoning_agent)
    graph.add_node("report", run_report_agent)

    graph.set_entry_point("perception")
    graph.add_edge("perception", "investigation")
    graph.add_edge("investigation", "reasoning")
    graph.add_edge("reasoning", "report")
    graph.add_edge("report", END)

    return graph.compile()


def run_rca_pipeline(scenario_file: str, hmi_screenshot_path: str | None = None) -> RCAState:
    """Convenience entry point: run the full pipeline for one scenario log."""
    app = build_rca_graph()
    initial_state: RCAState = {
        "scenario_file": scenario_file,
        "hmi_screenshot_path": hmi_screenshot_path,
        "errors": [],
    }
    return app.invoke(initial_state)
