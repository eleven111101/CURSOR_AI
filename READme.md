# rca_agents — AI Multi-Agent Root Cause Analysis Pipeline

A 4-agent LangGraph pipeline that investigates ADAS software failures by combining
a runtime log, an HMI screenshot, a Neo4j code-property-graph (built from Joern),
and direct source-code lookup — then produces a structured, file-and-function-level
RCA report.

**Pipeline:** Runtime Log + HMI Screenshot → **Perception** → **Investigation**
(Neo4j + source) → **Reasoning** (root cause + hop-by-hop trace) → **Report**
(Markdown)

Status: working POC, validated against 3 real scenarios on the actual ADAS
codebase (see [Validated Scenarios](#validated-scenarios) below).

---

## Install

```bash
cd ADAS_Project          # or wherever this rca_agents/ folder sits alongside your ADAS source
pip install -r rca_agents/requirements.txt
cp rca_agents/.env.example rca_agents/.env
```

Edit `rca_agents/.env`:

```env
OPENAI_API_KEY=sk-...
OPENAI_MODEL=gpt-4o          # or gpt-5.4 / another vision-capable model

NEO4J_URI=bolt://localhost:7687
NEO4J_USER=neo4j
NEO4J_PASSWORD=<your real password>

ADAS_PROJECT_ROOT=.          # or an absolute path if you run from elsewhere
SOURCE_DIR=ADAS              # set to the folder that actually contains your .c/.h files
OUTPUT_DIR=output
LOGS_DIR=execution/logs
```

Run every command from the folder that directly contains `requirements.txt`,
`.env`, and the inner `rca_agents/` package — not from inside that inner package
itself. If your `.c`/`.h` source sits directly in a folder without a nested
`ADAS/` subfolder, set `SOURCE_DIR` accordingly (see Troubleshooting).

## Run

```bash
python -m rca_agents.main --scenario run_case1.json --hmi screenshots/run_case1.png
python -m rca_agents.main --scenario run_case2.json --hmi screenshots/run_case2.png --output reports/run_case2_report.md
```

`--scenario` is required and must match a filename under `LOGS_DIR`. `--hmi` is
optional — if given, the Perception Agent sends the image to a vision-capable
model alongside the log. `--output` writes the Markdown report to a file
(parent folders are created automatically); without it, the report prints to
stdout only and is not saved anywhere.

---

## Agent Architecture

Four agents, wired as a fixed sequential LangGraph (`graph.py`):

```
Runtime Log + HMI Screenshot
        │
        ▼
┌─────────────────┐
│ Perception Agent │  reads the runtime log JSON + (optionally) the HMI
│ (perception.py)  │  screenshot via vision, produces a normalized
└────────┬─────────┘  Observation: summary + flagged_signals
         ▼
┌────────────────────┐
│ Investigation Agent │  LLM with tool-calling (up to 8 hops). Queries
│ (investigation.py)  │  Neo4j (call graph, variable reads/writes) and
└────────┬─────────────┘ pulls real source snippets. Builds an
         ▼                EvidencePackage of numbered, tool-tagged hops.
┌─────────────────┐
│ Reasoning Agent  │  Combines all evidence into a RootCause: hypothesis,
│ (reasoning.py)   │  confidence, responsible file/function, and an
└────────┬─────────┘  ordered hop-by-hop call_trace with exactly one
         ▼             hop marked as the fault point.
┌────────────────┐
│ Report Agent    │  Renders everything into a 9-section Markdown report
│ (report.py)     │  (see Report Format below).
└────────┬────────┘
         ▼
   Final RCA Report
```

Each agent is a plain function `(RCAState) -> RCAState` (shared state schema in
`state.py`); LangGraph just sequences them.

### Investigation Agent tools (`agents/investigation.py`)

| Tool | Purpose |
|---|---|
| `find_callers` / `find_callees` | Call-graph neighbors of a function |
| `execution_path_from_main` | Full reachable call path from `main()` |
| `call_chain_between` | Shortest call path between two named functions |
| `variable_readers` / `variable_writers` | Which functions read/write a variable or `#define` constant, plus the actual reading expressions and controlling if-statements |
| `get_source_snippet` | Real source code for a function, pulled from the actual `.c` files (regex-matched to the definition, not a call site) |

**Reliability features already built in:**
- If a Neo4j call fails with an auth/connection error, the agent detects it, stops retrying Neo4j for the rest of that run, and switches to source-code lookup instead of burning its entire tool-call budget on a dead connection.
- Every tool call in a batch gets a response before the next LLM turn — OpenAI rejects assistant messages with unanswered `tool_call_id`s, so this ordering is enforced in code.
- At least one `get_source_snippet` call is enforced (not just prompted) before the agent is allowed to finish, so the Reasoning/Report agents always have real file-path evidence to work with, even in a "nothing's wrong" case.

---

## Neo4j / Joern Graph Schema (confirmed against the real project graph)

This schema was reverse-engineered by directly querying the project's Neo4j
instance — not assumed. If you regenerate the CPG with a different Joern
version or extraction script, re-verify with the Cypher queries below.

**Node labels:** every node carries a base `CpgNode` label plus a specific type:
`Function`, `Parameter`, `Variable`, `Identifier`, `Call`, `ControlStructure`,
`Member`, `Type`, `File`, `Module`, `Literal`, `Include`, `MethodReturn`.

**`Function` nodes** have properties `name`, `fullName`, `code` — **no** `file`,
`lineNumber`, or `signature` (those don't exist in this graph; don't add them
back into queries).

**Struct-level names** (`sensorData`, `febData`, `hmiData`) and plain scalars
(`risk`, `speed`, `distance`) are `Variable` nodes.

**Struct fields** (`riskScore`, `warningRequest`, `brakeRequest`, `warningLamp`,
etc.) are **not** `Variable` nodes — they don't exist there at all. Each usage
site of a field is its own `Identifier` node with that field's name.

**`#define` macros/constants** (`FEATURE_ENABLE`, `WARNING_RISK`, `BRAKE_RISK`)
are modeled differently again: as their own `Function` node (the macro
declaration) plus a `Call` node at every usage site — never as `Identifier`
nodes. `neo4j_client.py` has an explicit fallback for this (see below).

**Confirmed relationships:**
```
(:Function)-[:CALLS]->(:Function)
(:Function)-[:USES_IDENTIFIER]->(:Identifier)      -- which function(s) touch an identifier
(:Call)-[:READS]->(:Identifier)                     -- an expression reading the identifier
(:Identifier)-[:DATA_FLOW]->(...)
(:Identifier)-[:DATA_DEPENDENCE]->(...)
(:Identifier)-[:CONTROL_DEPENDENCE]->(:ControlStructure)
(:Type)-[:HAS_MEMBER]->(:Member)                    -- struct field declarations only, not usage sites
```

**Verification queries** (run in Neo4j Browser if this ever needs re-checking):
```cypher
MATCH (i:Identifier {name:'riskScore'}) RETURN i.name, i.code LIMIT 5
MATCH (f:Function)-[:USES_IDENTIFIER]->(i:Identifier {name:'riskScore'}) RETURN f.name
MATCH (n) WHERE n.name = 'FEATURE_ENABLE' OR n.code CONTAINS 'FEATURE_ENABLE'
RETURN labels(n), n.name, n.code LIMIT 15
```

`variable_readers`/`variable_writers` in `neo4j_client.py` query `Identifier`
nodes first; if that returns nothing (the macro case), they fall back to
matching `Function`/`Call` nodes by literal code-text containment. This is what
lets `variable_readers("FEATURE_ENABLE")` correctly find `EvaluateWarning` and
`EvaluateBrake` even though `FEATURE_ENABLE` has no `Identifier` node.

---

## Report Format

The Report Agent (`agents/report.py`) always produces these 9 sections:

1. **Summary** — what happened, root cause, confidence
2. **Files & Functions Involved** — every real file/function from the evidence, with its role
3. **Code Trace (hop by hop)** — Hop | File | Function | Variables | Expected | Actual | Fault point?, one row per hop in the call_trace, fault row marked
4. **Root Cause** — concrete explanation naming the responsible file/function
5. **Expected vs Actual (Case Comparison)** — side-by-side diff table
6. **Supporting Evidence** — bullets citing specific hop numbers
7. **Affected Component(s)** — which SWC(s)/files
8. **Investigation Effort** — exact hop/tool-call count and breakdown (e.g. "9 hops: 3x variable_readers, 4x get_source_snippet, 2x find_callers")
9. **Recommended Corrective Actions** — specific to the responsible file/function

The Report Agent is instructed never to invent file paths, function names, or
evidence not present in what the Investigation/Reasoning agents actually found.

---

## Validated Scenarios

Three scenarios, each a real runtime log + a generated HMI dashboard screenshot,
all traced against the actual `ADAS_Project` source (not synthetic/invented
values — every `riskScore`, `warningRequest`, etc. was computed by hand through
the real `CalculateRiskScore` / `EvaluateWarning` / `EvaluateBrake` formulas
before being written into the log).

| | Case 1 — `run_case1` | Case 2 — `run_case2` | Case 3 — `run_case3` |
|---|---|---|---|
| Speed / Distance | 72 km/h / 12 m | 72 km/h / 12 m | 65 km/h / 6 m |
| `riskScore` | 80 | 80 | 100 |
| `FEATURE_ENABLE` | 1 | **0** | 1 |
| `warningRequest` | 1 | 0 | 1 |
| `brakeRequest` | 0 | 0 | 1 |
| Backend decision | correct | correct logic, but gated OFF | correct |
| HMI reflects it? | yes | no (expected — matches the gate) | **no — bug** |
| Actual fault | none | `EvaluateWarning` (`ADAS/src/warning.c`) | `UpdateBrakeLamp` (`ADAS/src/display.c`) |
| Fault type | — | config/feature-flag gating suppresses a correct decision | no-op function; `HMIData` has no `brakeLamp` field to write to |
| Pipeline result | no fault found (correct) | high confidence, correct root cause | high confidence, correct root cause |

**Case 1** — baseline "everything worked" case, used to confirm the pipeline
doesn't hallucinate a fault where none exists.

**Case 2** — `EvaluateWarning()`/`EvaluateBrake()` both check
`if(FEATURE_ENABLE == OFF) return OFF;` *before* checking the risk threshold.
With `FEATURE_ENABLE=0`, a genuinely correct risk computation (`riskScore=80`,
above the `WARNING_RISK=60` threshold) never reaches the threshold check at
all. The pipeline correctly identified `EvaluateWarning` as the fault point and
cleared every downstream HMI function (`UpdateWarningLamp`, `UpdateBuzzer`,
`UpdateDisplay`) as innocent, since they're just faithfully propagating the
already-suppressed signal.

**Case 3** — `EvaluateBrake()` correctly computes `brakeRequest=1` at
`riskScore=100` (`FEATURE_ENABLE=1` this time, no gating issue), and the
actuator log confirms `"Brake Actuator : APPLIED"`. But `UpdateBrakeLamp()` in
`display.c` is `(void)brakeRequest;` — a literal no-op — and `HMIData` (in
`common.h`) has no `brakeLamp` field at all. The pipeline traced 9 hops,
correctly cleared `EvaluateWarning`, `EvaluateBrake`, `UpdateDisplay`,
`UpdateBuzzer`, and `UpdateWarningLamp` (all genuinely correct in this run),
and isolated `UpdateBrakeLamp` as the sole fault — a structurally different
bug class from Case 2 (missing HMI wiring vs. config gating), which is good
evidence the pipeline generalizes rather than pattern-matching one bug type.

---

## Why graph-backed evidence instead of dumping the whole codebase into context

The Investigation Agent never reads the full codebase. It queries Neo4j for
exactly the functions/variables relevant to the observation's flagged signals,
and pulls source snippets only for the handful of functions the graph
identifies as relevant — typically 4-9 targeted hops per run, not "index every
file." This is why cost stays low and accuracy stays high as the codebase
grows: query cost is a function of how many relevant nodes exist, not how many
total lines of code exist. All 3 validated runs above stayed well under $1 in
API cost, including Case 3's 9-hop investigation.

---

## Troubleshooting

**`No module named rca_agents.main`**
You're running from the wrong directory. The package layout is
`rca_agents/rca_agents/main.py` — `cd` into the *outer* `rca_agents/` folder
(the one containing `requirements.txt` and `.env`), not the inner package.

**`FileNotFoundError` when using `--output`**
Fixed as of this version — `main.py` now creates parent directories
automatically before writing. If you're on an older copy, either update it or
`mkdir` the target folder yourself first.

**Investigation Agent burns all its hops on Neo4j errors**
Check `NEO4J_PASSWORD` in `.env`. The agent detects auth/connection failures
and stops retrying Neo4j after the first one (switching to source-code lookup
instead), but if your credentials are simply wrong, fix them and rerun — you
should see real evidence instead of "authentication failed" in every hop.

**`variable_readers`/`variable_writers` return empty for a struct field**
Struct fields aren't `Variable` nodes in this graph — see the Schema section
above. This is already handled by `neo4j_client.py`; if it recurs after a CPG
regeneration, re-run the verification queries in that section.

**`variable_readers`/`variable_writers` return empty for a `#define` constant
(e.g. `FEATURE_ENABLE`)**
Also handled — see the macro fallback described in the Schema section.

**`openai.BadRequestError: 400 - An assistant message with 'tool_calls' must be
followed by tool messages...`**
This was a message-ordering bug (fixed as of this version): if the model fired
multiple tool calls in one batch and one hit an error mid-batch, an out-of-band
message used to get inserted before every `tool_call_id` in that batch had a
response, which OpenAI's API rejects. All tool responses in a batch are now
guaranteed contiguous before any other message is appended.

**`Function` metadata missing file/line info**
This graph's `Function` nodes genuinely don't carry `file`/`lineNumber`/
`signature` properties — only `name`, `fullName`, `code`. File paths shown in
reports come from `get_source_snippet`'s direct filesystem lookup instead, not
from Neo4j.

---

## Known Simplifications (POC scope)

- `find_function_in_source` uses a regex match over `.c` files to find function
  *definitions* (not call sites) — solid for this codebase's straightforward
  style, but not a full parser; unusual formatting could still trip it up.
- `variable_writers` is best-effort: it looks for assignment-style expressions
  near a variable's usage sites, but this graph has no confirmed explicit
  `WRITES` edge, so it can under-report writes in more complex code.
- Tool-calling is capped at `MAX_TOOL_CALLS = 8` per Investigation Agent run
  (raise it in `investigation.py` for scenarios needing deeper traversal).
- No persistence layer — each CLI run is stateless. `reports/*.md` is the only
  durable output; there's no run history/database.
- Single LLM model for all 4 agents (set once via `OPENAI_MODEL` in `.env`).
  Splitting per-agent models (e.g. a stronger reasoning model for the
  Reasoning Agent, a cheaper model for Report) is a straightforward follow-up
  if cost/quality tuning becomes a priority.

## Extending

- **New scenario:** drop a `.json` into `LOGS_DIR` with the same shape
  (`sensorData`, `febData`, `hmiData`, `config`, `actuator`) and a matching
  screenshot, then `--scenario <filename>.json --hmi <path>`.
- **Swap LLM provider:** only `agents/*.py` import `ChatOpenAI` from
  `langchain_openai`; swap for `ChatAnthropic` or another LangChain chat model
  — `state.py`, `graph.py`, and the Neo4j/file tools are provider-agnostic.
- **Add a Critic/Verification agent:** insert a node between `reasoning` and
  `report` in `graph.py` that checks `root_cause` against `evidence_package`
  before the report is written.
- **Batch mode:** run all scenarios in one command instead of one at a time —
  not yet implemented, would iterate `LOGS_DIR` and pair each log with its
  matching screenshot by filename.

---

## Owner

**gravity_AI**

## Citation

If referencing this project, please cite as:

```
gravity_AI. "AI-Assisted Root Cause Analysis for Embedded C ADAS Software"
(rca_agents), 2026.
```