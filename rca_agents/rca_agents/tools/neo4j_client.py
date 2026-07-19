"""
Thin wrapper around the Neo4j knowledge graph built from the Joern/GSCEF
JSON extraction (functions, calls, reads/writes, control & data flow).

Schema CONFIRMED against the actual project graph (not guessed):

  Nodes are multi-labeled: every node has base label "CpgNode" plus a
  specific type label: Function, Parameter, Variable, Identifier, Call,
  ControlStructure, Member, Type, File, Module, Literal, Include,
  MethodReturn.

  Function nodes have properties: name, fullName, code
    (NOT file, lineNumber, or signature -- those don't exist in this graph)

  Struct-level names (sensorData, febData, hmiData) and plain scalars
  (risk, speed, distance) are Variable nodes.

  Struct FIELDS (riskScore, warningRequest, brakeRequest, warningLamp,
  etc.) are NOT Variable nodes and are NOT reachable via Variable lookup.
  Each usage site of a field shows up as its own Identifier node with
  that field's name (e.g. two Identifier{name:"riskScore"} nodes for two
  usage sites in the source).

  Confirmed relationships:
    (:Function)-[:CALLS]->(:Function)
    (:Function)-[:USES_IDENTIFIER]->(:Identifier)   -- which function(s) touch a given identifier
    (:Call)-[:READS]->(:Identifier)                  -- an expression/operator Call reading the identifier
    (:Identifier)-[:DATA_FLOW]->(...)                -- data flow edges (Call, Parameter, etc.)
    (:Identifier)-[:DATA_DEPENDENCE]->(...)
    (:ControlStructure)-[:GATED_BY]<-(:Identifier) / (:Identifier)-[:CONTROL_DEPENDENCE]->(:ControlStructure)
    (:Type)-[:HAS_MEMBER]->(:Member)                  -- struct field declarations (not usage sites)

  No explicit WRITES edge was confirmed to exist from a simple pattern
  match; if your CPG build does emit WRITES, run_cypher() can be used
  directly. variable_writers() below infers writes via CONTROL/DATA
  relationships and by convention (assignment Calls), and is best-effort.
"""

from __future__ import annotations

from neo4j import GraphDatabase

from rca_agents.config import NEO4J_PASSWORD, NEO4J_URI, NEO4J_USER


class Neo4jClient:
    def __init__(self, uri: str = NEO4J_URI, user: str = NEO4J_USER, password: str = NEO4J_PASSWORD):
        self._driver = GraphDatabase.driver(uri, auth=(user, password))

    def close(self) -> None:
        self._driver.close()

    def _run(self, query: str, **params) -> list[dict]:
        with self._driver.session() as session:
            result = session.run(query, **params)
            return [record.data() for record in result]

    # ---- Core queries used by the Investigation Agent ----

    def find_callers(self, function_name: str) -> list[dict]:
        """Who calls this function? (file/line not available in this graph -- name only)"""
        query = """
        MATCH (caller:Function)-[:CALLS]->(callee:Function {name: $name})
        RETURN caller.name AS caller, caller.fullName AS callerFullName
        """
        return self._run(query, name=function_name)

    def find_callees(self, function_name: str) -> list[dict]:
        """What does this function call?"""
        query = """
        MATCH (f:Function {name: $name})-[:CALLS]->(callee:Function)
        RETURN callee.name AS callee, callee.fullName AS calleeFullName
        """
        return self._run(query, name=function_name)

    def execution_path_from_main(self, limit: int = 200, max_depth: int = 10) -> list[dict]:
        """Full reachable call path starting at main(), depth-capped to avoid runaway traversal."""
        query = f"""
        MATCH p = (m:Function {{name: "main"}})-[:CALLS*1..{max_depth}]->(f:Function)
        RETURN [n IN nodes(p) | n.name] AS path
        LIMIT $limit
        """
        return self._run(query, limit=limit)

    def which_functions_use_identifier(self, identifier_name: str) -> list[dict]:
        """
        Which functions reference/touch a given identifier (e.g. riskScore,
        warningRequest, FEATURE_ENABLE)? This is the primary "who
        reads/writes this variable" signal in this graph, since struct
        fields are Identifier nodes, not Variable nodes, and the reliable
        edge into a function is USES_IDENTIFIER.

        Also handles #define macros/constants (e.g. FEATURE_ENABLE,
        WARNING_RISK, BRAKE_RISK): these are NOT Identifier nodes in this
        graph -- they show up as their own Function node (the macro
        declaration) and as Call nodes at each usage site, with no
        USES_IDENTIFIER edge. When the Identifier-based search finds
        nothing, this falls back to matching any Function whose source
        code text literally contains the name, which reliably catches
        macro usage regardless of how Joern modeled the edges.
        """
        query = """
        MATCH (f:Function)-[:USES_IDENTIFIER]->(i:Identifier {name: $name})
        RETURN DISTINCT f.name AS function, i.code AS identifierCode
        """
        results = self._run(query, name=identifier_name)
        if results:
            return results

        # Fallback for macros/constants (FEATURE_ENABLE, WARNING_RISK, etc.)
        # that aren't modeled as Identifier nodes: find real functions whose
        # code body literally mentions the name, excluding the macro's own
        # #define declaration node.
        fallback_query = """
        MATCH (f:Function)
        WHERE f.code CONTAINS $name AND NOT f.code STARTS WITH '#define'
        RETURN DISTINCT f.name AS function, f.code AS identifierCode
        """
        return self._run(fallback_query, name=identifier_name)

    def identifier_usage_context(self, identifier_name: str, limit: int = 20) -> list[dict]:
        """
        Show the actual expressions (Call nodes) that read this identifier,
        e.g. 'riskScore >= WARNING_RISK', plus any controlling IF statement
        (GATED_BY / CONTROL_DEPENDENCE), which is exactly the evidence
        needed to see threshold/gating logic around a variable.

        Falls back to matching Call nodes directly by name for macros/
        constants (FEATURE_ENABLE, WARNING_RISK, BRAKE_RISK) that aren't
        modeled as Identifier nodes -- their usage sites appear as their
        own Call node (e.g. name='FEATURE_ENABLE', code='FEATURE_ENABLE'
        inside a parent Call like '<operator>.equals' with code
        'FEATURE_ENABLE == OFF').
        """
        query = """
        MATCH (i:Identifier {name: $name})
        OPTIONAL MATCH (readCall:Call)-[:READS]->(i)
        OPTIONAL MATCH (i)-[:CONTROL_DEPENDENCE]->(cs:ControlStructure)
        RETURN DISTINCT
            readCall.code AS readingExpression,
            cs.code AS controllingStatement
        LIMIT $limit
        """
        results = self._run(query, name=identifier_name, limit=limit)
        has_real_data = any(r.get("readingExpression") or r.get("controllingStatement") for r in results)
        if has_real_data:
            return results

        # Fallback: macro/constant usage sites are Call nodes, not
        # Identifier nodes. Find the parent expression and any controlling
        # IF statement around them instead.
        fallback_query = """
        MATCH (c:Call {name: $name})
        OPTIONAL MATCH (parent:Call)-[:AST_CHILD]->(c)
        OPTIONAL MATCH (cs:ControlStructure)-[:AST_CHILD]->(parent)
        RETURN DISTINCT
            coalesce(parent.code, c.code) AS readingExpression,
            cs.code AS controllingStatement
        LIMIT $limit
        """
        return self._run(fallback_query, name=identifier_name, limit=limit)

    def variable_readers(self, variable_name: str) -> dict:
        """
        Which functions read a given variable/field (e.g. riskScore,
        warningRequest)? Backed by USES_IDENTIFIER + READS since this
        graph does not model struct fields as Variable nodes. Returns a
        single dict: {"functions": [...], "reading_expressions": [...]}
        rather than one row per function, since the expressions/context
        apply to the identifier as a whole, not per-function.
        """
        functions = self.which_functions_use_identifier(variable_name)
        context = self.identifier_usage_context(variable_name)
        return {
            "functions": [f["function"] for f in functions],
            "reading_expressions": [
                c["readingExpression"] for c in context if c.get("readingExpression")
            ],
            "controlling_statements": [
                c["controllingStatement"] for c in context if c.get("controllingStatement")
            ],
        }

    def variable_writers(self, variable_name: str) -> list[dict]:
        """
        Which functions write/assign a given variable/field? Best-effort:
        looks for an assignment-style Call (<operator>.assignment) that
        involves this identifier, within functions that use it. Falls back
        to the same USES_IDENTIFIER set as variable_readers if no explicit
        assignment pattern is found -- still useful evidence (it tells you
        which functions are in scope), just not write-specific.
        """
        query = """
        MATCH (f:Function)-[:USES_IDENTIFIER]->(i:Identifier {name: $name})
        OPTIONAL MATCH (assignCall:Call)-[:DATA_FLOW]->(i)
        WHERE assignCall IS NULL
           OR assignCall.name CONTAINS 'assignment'
           OR assignCall.code CONTAINS '='
        RETURN DISTINCT f.name AS function, assignCall.code AS assignmentExpression
        """
        return self._run(query, name=variable_name)

    def call_chain_between(self, from_func: str, to_func: str, max_depth: int = 8) -> list[dict]:
        """Shortest call path between two functions, if any."""
        query = f"""
        MATCH p = shortestPath(
            (a:Function {{name: $from_func}})-[:CALLS*1..{max_depth}]->(b:Function {{name: $to_func}})
        )
        RETURN [n IN nodes(p) | n.name] AS path
        """
        return self._run(query, from_func=from_func, to_func=to_func)

    def function_metadata(self, function_name: str) -> list[dict]:
        """
        Basic metadata for a function node. This graph only has name,
        fullName, and code on Function nodes -- no file/line/signature.
        """
        query = """
        MATCH (f:Function {name: $name})
        RETURN f.name AS name, f.fullName AS fullName, f.code AS code
        """
        return self._run(query, name=function_name)

    # ---- Raw escape hatch ----

    def run_cypher(self, query: str, **params) -> list[dict]:
        return self._run(query, **params)