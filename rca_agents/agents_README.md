# rca_agents

4-agent LangGraph pipeline implementing the "AI Agent Pipeline (Next Phase)" from the
project README: **Perception → Investigation → Reasoning → Report**.

## Install

Drop this `rca_agents/` folder into your `ADAS_Project/` root (replacing the empty
placeholder folder), then:

```bash
cd ADAS_Project
pip install -r rca_agents/requirements.txt
cp rca_agents/.env.example rca_agents/.env
# edit rca_agents/.env: set OPENAI_API_KEY, and Neo4j creds if not using defaults
```

Run from `ADAS_Project/` so relative paths (`output/`, `execution/logs/`, `ADAS/`)
resolve correctly, or set `ADAS_PROJECT_ROOT` in `.env` to an absolute path.

## Run

```bash
python -m rca_agents.main --scenario run_case1.json --hmi screenshots/run_case1.png
python -m rca_agents.main --scenario run_case2.json --hmi screenshots/run_case2.png --output reports/run_case2_rca.md
```

`--scenario` is required and must match a filename under `execution/logs/`
(`run_case1.json`, `run_case2.json`, or any new scenario log you add in the same shape).

## Architecture

```
rca_agents/
├── state.py                  # shared LangGraph state (Observation, EvidencePackage, RootCause)
├── config.py                 # env vars, paths
├── graph.py                  # LangGraph wiring: perception -> investigation -> reasoning -> report
├── main.py                   # CLI entry point
├── tools/
│   ├── neo4j_client.py       # wraps the Cypher queries already defined in the project (CALLS, READS, WRITES)
│   └── file_tools.py         # runtime log loading, source snippet lookup, HMI screenshot hook
└── agents/
    ├── perception.py         # parses runtime log -> normalized Observation
    ├── investigation.py      # LLM + tool-calling agent: queries Neo4j & source -> EvidencePackage
    ├── reasoning.py          # combines evidence -> RootCause hypothesis
    └── report.py             # generates final Markdown RCA report
```

Each agent is a plain Python function `(RCAState) -> RCAState`, wired together as
LangGraph nodes in `graph.py`. This mirrors the pipeline diagram already in the
project README:

```
Runtime Log + HMI Screenshot
        -> Perception Agent -> Observation
        -> Investigation Agent (Neo4j / Runtime Log / Source Code) -> Evidence Package
        -> Reasoning Agent -> Root Cause
        -> Report Agent -> Final RCA Report
```

## Included Test Scenarios

Two runtime log + HMI screenshot pairs are included under `execution/logs/` and
`screenshots/`, both at identical driving conditions (72 km/h, 12 m following
distance) so the only variable between them is the outcome -- this makes the
comparison itself the evidence:

| Scenario | riskScore | warningRequest | Warning Lamp / Buzzer | FEATURE_ENABLE |
|---|---|---|---|---|
| `run_case1` (expected) | 80 | 1 | ON / ON | 1 |
| `run_case2` (issue) | 80 | 0 | OFF / OFF | 0 |

Traced against the real thresholds in `config.h` (`WARNING_RISK=60`,
`BRAKE_RISK=85`): at speed=72, distance=12, `CalculateRiskScore()` computes
`riskScore=80`, which is above `WARNING_RISK` -- so `EvaluateWarning()` should
return ON in both cases. `run_case2` reproduces the issue by setting
`FEATURE_ENABLE=0`, which makes `EvaluateWarning()` short-circuit to OFF
regardless of `riskScore`. This is the intended root cause for the pipeline to
find: **not a sensor or risk-calculation fault, but the feature flag disabling
the warning path entirely.**

Run both:

```bash
python -m rca_agents.main --scenario run_case1.json --hmi screenshots/run_case1.png
python -m rca_agents.main --scenario run_case2.json --hmi screenshots/run_case2.png
```

The screenshots are cropped from a single reference comparison image (dashboard
+ matching detail table per case: Speed, Distance, Warning Lamp, Audible Alert,
Display Message). Swap in new screenshots by replacing these two files with the
same filenames.

## Report Format

The final report (`state.final_report`, written by the Report Agent) is Markdown
with these sections, in order:

1. **Summary** -- what happened, root cause, confidence, in a few sentences.
2. **Files & Functions Involved** -- a table of every file/function that appeared
   in the evidence, with its role (e.g. "gates warning lamp on FEATURE_ENABLE").
3. **Code Trace (hop by hop)** -- the ordered call trace as a table: Hop | File |
   Function | Variables | Expected | Actual | Fault point?. The row where behavior
   first diverges from expected is marked as the fault point.
4. **Root Cause** -- the concrete explanation, naming the responsible file/function.
5. **Expected vs Actual (Case Comparison)** -- for a 2-case run (like `run_case1`
   vs `run_case2`), a side-by-side table of only the fields that differ between
   cases.
6. **Supporting Evidence** -- bullets citing specific hop numbers from the
   Investigation Agent (e.g. "Hop 3: get_source_snippet(EvaluateWarning) confirmed
   the FEATURE_ENABLE gate runs before the risk check").
7. **Affected Component(s)** -- which SWC(s)/files.
8. **Investigation Effort** -- how many hops/tool calls were used and which tools
   (e.g. "4 hops: 1x variable_writers, 2x get_source_snippet, 1x find_callers").
9. **Recommended Corrective Actions** -- specific to the responsible file/function.

This structure is enforced by the Report Agent's system prompt in
`rca_agents/agents/report.py`, and is fed real data: the Reasoning Agent
(`rca_agents/agents/reasoning.py`) is responsible for producing the ordered
`call_trace` (file/function/variables/expected/actual per hop, with exactly one
hop marked `is_fault_point: true`), and the Investigation Agent tags every piece
of evidence it gathers with a `hop` number and the `tool` that produced it, so
the report can cite exact hop numbers and give an accurate hop-count summary.

## HMI screenshot support

Pass `--hmi path/to/screenshot.png` and the Perception Agent sends the image
directly to the LLM (GPT-4o, vision-capable) alongside the log JSON, in the same
call. The model is asked to describe what's actually lit/shown on the HMI and flag
any mismatch against what the log signals imply (e.g. log says `warningRequest = 1`
but the lamp in the photo looks off). Supported formats: `.png`, `.jpg`, `.jpeg`,
`.webp`, `.gif`. If the path doesn't exist or isn't a supported type, the agent
just proceeds log-only.

## Troubleshooting

**Investigation Agent burns all its hops on Neo4j errors, report says "authentication failures"**
Your `NEO4J_PASSWORD` (or user/URI) in `.env` is wrong, or Neo4j isn't running. The
Investigation Agent now detects an auth/connection failure on the *first* Neo4j call,
stops calling Neo4j tools for the rest of that run, and switches to `get_source_snippet`
so remaining hops are still useful. Fix the credentials in `.env` and re-run --
confidence should go from "medium" (log-only reasoning) to "high" (code-confirmed)
once Neo4j evidence is actually available.

**`No module named rca_agents.main`**
You're running `python -m rca_agents.main` from the wrong directory. The package layout
is `rca_agents/rca_agents/main.py` -- you must `cd` into the *outer* `rca_agents/` folder
(the one containing `requirements.txt`, `.env`, and the inner `rca_agents/` package)
before running the command.

**Source files not found / `get_source_snippet` returns "not found in source tree"**
Check `SOURCE_DIR` in `.env` matches where your `.c`/`.h` files actually live relative
to `ADAS_PROJECT_ROOT`. If your source sits directly in `ADAS_Project/` (no nested
`ADAS/` folder), set `SOURCE_DIR=ADAS_Project` rather than the default `ADAS`.

## Notes on the Neo4j schema assumed

`tools/neo4j_client.py` assumes the graph schema already established by your
GSCEF/Joern extraction and load script (`load_to_neo4j.py`):

- `(:Function)-[:CALLS]->(:Function)`
- `(x)-[:READS]->(v)` / `(x)-[:WRITES]->(v)` where `v` is a variable node
- `Function` nodes carry `name`, `file`, and ideally `lineNumber` / `signature`

If your actual property names differ (e.g. `f.filename` instead of `f.file`), adjust
the `RETURN` clauses in `neo4j_client.py` to match — the rest of the pipeline is
schema-agnostic since it only consumes whatever JSON the tool functions return.

## Extending

- **New scenario**: drop a new `.json` into `execution/logs/` with the same
  top-level shape (`sensorData`, `febData`, `hmiData`, `config`) and pass
  `--scenario <filename>.json`.
- **Swap LLM provider**: only `agents/*.py` import `ChatOpenAI` from
  `langchain_openai`; swap for `ChatAnthropic` or another LangChain chat model and
  update `config.py` accordingly — `state.py`, `graph.py`, and the tools are provider-agnostic.
- **Add a Critic/Verification agent** later: insert a node between `reasoning` and
  `report` in `graph.py` that checks `root_cause` against `evidence_package` before
  the report is written.

## Known simplifications (POC scope)

- `find_function_in_source` does a plain string search over `.c` files rather than
  using Joern/CPG for source lookup — fine for a POC evidence package, but not as
  precise as a proper CPG-backed lookup.
- The Investigation Agent's tool-calling loop is capped at `MAX_TOOL_CALLS = 8`
  (in `agents/investigation.py`) to bound cost/latency; raise it if scenarios need
  deeper graph traversal.
- No persistence layer — each CLI run is stateless. Add a `reports/` output
  convention or a DB if you want to track RCA runs over time.
