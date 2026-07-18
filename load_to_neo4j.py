#!/usr/bin/env python3
"""
load_to_neo4j.py — loads GSCEF run.sc output JSON files into a running Neo4j instance.

Reads: metadata.json, symbols.json, dependencies.json, callgraph.json,
       ast.json, cfg.json, pdg.json, dataflow.json
(metrics.json / summary.json are NOT graph-shaped and are skipped here —
 metrics.perMethod can be loaded separately later if you want per-method
 stats as node properties.)

Every node gets an extra generic label `CpgNode` (in addition to its real
label, e.g. Function/Variable/Call) so edges can be matched purely by `id`
without needing to know the source/destination's specific label. This is
safe because Joern node IDs are unique across the whole CPG.

Usage:
    pip install neo4j
    python load_to_neo4j.py --output-dir "/path/to/output" --uri bolt://localhost:7687 --user neo4j

You'll be prompted for the password (not passed on the command line, so it
doesn't end up in shell history). Safe to re-run — everything uses MERGE,
so re-running after a fresh `run.sc` extraction just updates/adds, never
duplicates.
"""

import argparse
import getpass
import json
import sys
import time
from pathlib import Path

try:
    from neo4j import GraphDatabase
except ImportError:
    print("Missing dependency. Install it first:\n    pip install neo4j")
    sys.exit(1)

# Files that contain a graph-shaped {"nodes": [...], "edges": [...]} payload.
GRAPH_FILES = [
    "metadata.json",
    "symbols.json",
    "dependencies.json",
    "callgraph.json",
    "ast.json",
    "cfg.json",
    "pdg.json",
    "dataflow.json",
]

BATCH_SIZE = 1000


def load_json(path: Path):
    if not path.exists():
        print(f"  [skip] {path.name} not found")
        return {"nodes": [], "edges": []}
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def node_props(n: dict) -> dict:
    """Flatten a GNode's top-level fields + its nested 'properties' dict
    into a single flat dict suitable for Cypher SET n += $props."""
    props = {
        "id": n.get("id"),
        "type": n.get("type"),
        "name": n.get("name"),
        "fullName": n.get("fullName"),
        "code": n.get("code"),
        "file": n.get("file"),
        "line": n.get("line"),
        "column": n.get("column"),
    }
    nested = n.get("properties") or {}
    for k, v in nested.items():
        props[k] = v
    return props


def edge_props(e: dict) -> dict:
    return e.get("properties") or {}


def collect_nodes_and_edges(output_dir: Path):
    """Read every graph file once, group nodes by label and edges by
    relationship type (Cypher needs a literal label/type, not a parameter,
    so grouping happens client-side)."""
    nodes_by_label = {}
    edges_by_rel = {}

    for fname in GRAPH_FILES:
        data = load_json(output_dir / fname)
        n_nodes = len(data.get("nodes", []))
        n_edges = len(data.get("edges", []))
        print(f"  {fname:20s} nodes={n_nodes:6d}  edges={n_edges:6d}")

        for n in data.get("nodes", []):
            label = n.get("label") or "Unknown"
            nodes_by_label.setdefault(label, []).append(node_props(n))

        for e in data.get("edges", []):
            rel = e.get("relationship") or "RELATED_TO"
            edges_by_rel.setdefault(rel, []).append({
                "source": e.get("source"),
                "destination": e.get("destination"),
                "props": edge_props(e),
            })

    return nodes_by_label, edges_by_rel


def batched(seq, size):
    for i in range(0, len(seq), size):
        yield seq[i:i + size]


def load_nodes(session, nodes_by_label: dict):
    print("\n--- Loading nodes ---")
    for label, rows in nodes_by_label.items():
        query = (
            f"UNWIND $rows AS row "
            f"MERGE (n:`{label}`:CpgNode {{id: row.id}}) "
            f"SET n += row"
        )
        total = 0
        for batch in batched(rows, BATCH_SIZE):
            session.run(query, rows=batch)
            total += len(batch)
        print(f"  {label:20s} {total:6d} nodes merged")


def load_edges(session, edges_by_rel: dict):
    print("\n--- Loading relationships ---")
    for rel, rows in edges_by_rel.items():
        query = (
            f"UNWIND $rows AS row "
            f"MATCH (a:CpgNode {{id: row.source}}) "
            f"MATCH (b:CpgNode {{id: row.destination}}) "
            f"MERGE (a)-[r:`{rel}`]->(b) "
            f"SET r += row.props"
        )
        total = 0
        matched = 0
        for batch in batched(rows, BATCH_SIZE):
            result = session.run(query, rows=batch)
            summary = result.consume()
            total += len(batch)
            matched += summary.counters.relationships_created
        print(f"  {rel:20s} {total:6d} edges processed  ({matched} newly created this run)")


def create_indexes(session, labels):
    print("\n--- Creating indexes ---")
    session.run("CREATE INDEX cpg_node_id IF NOT EXISTS FOR (n:CpgNode) ON (n.id)")
    print("  index on :CpgNode(id) ensured")
    for label in labels:
        # Safe: label names come from our own extractor's fixed set, not user input.
        session.run(f"CREATE INDEX IF NOT EXISTS FOR (n:`{label}`) ON (n.id)")
    print(f"  per-label id indexes ensured for {len(labels)} labels")


def main():
    parser = argparse.ArgumentParser(description="Load GSCEF output JSON into Neo4j")
    parser.add_argument("--output-dir", required=True, help="Path to the run.sc output/ folder")
    parser.add_argument("--uri", default="bolt://localhost:7687")
    parser.add_argument("--user", default="neo4j")
    parser.add_argument("--password", default=None, help="If omitted, you'll be prompted securely")
    args = parser.parse_args()

    output_dir = Path(args.output_dir)
    if not output_dir.exists():
        print(f"ERROR: output dir not found: {output_dir}")
        sys.exit(1)

    password = args.password or getpass.getpass(f"Neo4j password for user '{args.user}': ")

    print(f"\nReading JSON from: {output_dir}")
    nodes_by_label, edges_by_rel = collect_nodes_and_edges(output_dir)

    total_nodes = sum(len(v) for v in nodes_by_label.values())
    total_edges = sum(len(v) for v in edges_by_rel.values())
    print(f"\nParsed: {total_nodes} nodes across {len(nodes_by_label)} labels, "
          f"{total_edges} edges across {len(edges_by_rel)} relationship types")

    print(f"\nConnecting to {args.uri} as {args.user} ...")
    driver = GraphDatabase.driver(args.uri, auth=(args.user, password))
    driver.verify_connectivity()
    print("Connected.")

    start = time.time()
    with driver.session() as session:
        create_indexes(session, nodes_by_label.keys())
        load_nodes(session, nodes_by_label)
        load_edges(session, edges_by_rel)

    driver.close()
    elapsed = time.time() - start
    print(f"\nDONE in {elapsed:.1f}s. Open http://localhost:7474 and try:")
    print('  MATCH (m:Function {name:"main"})-[:CALLS]->(x) RETURN m, x')


if __name__ == "__main__":
    main()