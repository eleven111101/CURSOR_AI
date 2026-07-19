"""
CLI entry point.

Usage:
    python -m rca_agents.main --scenario run_003.json
    python -m rca_agents.main --scenario run_002.json --hmi path/to/screenshot.png
    python -m rca_agents.main --scenario run_003.json --output report.md
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from rca_agents.config import require_openai_key
from rca_agents.graph import run_rca_pipeline


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run the AI multi-agent RCA pipeline on a runtime scenario log.")
    parser.add_argument(
        "--scenario",
        required=True,
        help="Runtime log filename under execution/logs/, e.g. run_001.json, run_002.json, run_003.json",
    )
    parser.add_argument(
        "--hmi",
        default=None,
        help="Optional path to an HMI screenshot (.png/.jpg/.jpeg/.webp/.gif) for this "
        "scenario. If given, the Perception Agent sends it to a vision-capable model "
        "and compares what's shown (lamps, buzzer, display) against the log signals.",
    )
    parser.add_argument(
        "--output",
        default=None,
        help="Optional path to write the final Markdown report. If omitted, prints to stdout.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    try:
        require_openai_key()
    except RuntimeError as e:
        print(f"Config error: {e}", file=sys.stderr)
        return 1

    print(f"Running RCA pipeline for scenario: {args.scenario}", file=sys.stderr)
    print("  [1/4] Perception Agent...", file=sys.stderr)
    print("  [2/4] Investigation Agent...", file=sys.stderr)
    print("  [3/4] Reasoning Agent...", file=sys.stderr)
    print("  [4/4] Report Agent...", file=sys.stderr)

    final_state = run_rca_pipeline(scenario_file=args.scenario, hmi_screenshot_path=args.hmi)

    report = final_state.get("final_report", "No report generated.")

    if args.output:
        output_path = Path(args.output)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(report, encoding="utf-8")
        print(f"Report written to {args.output}", file=sys.stderr)
    else:
        print(report)

    # Also surface the raw root-cause JSON on stderr for debugging/inspection
    root_cause = final_state.get("root_cause", {})
    print("\n--- Root Cause (structured) ---", file=sys.stderr)
    print(root_cause, file=sys.stderr)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())