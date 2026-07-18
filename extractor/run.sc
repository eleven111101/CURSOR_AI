/* =====================================================================
   GSCEF — Generic Semantic CPG Extraction Framework
   run.sc — single-file, modular, Neo4j-ready JSON export

   Target runtime : Joern 4.0.583
   Usage:
     joern --script run.sc --import ../ADAS/cpg.bin

   `cpg` is injected automatically by Joern via --import. This script
   only CONSUMES an existing CPG — it never creates or re-parses one.

   -------------------------------------------------------------------
   HONESTY / CONFIDENCE NOTES (read before running):

   - Sections built on long-stable CPGQL steps (method, call, parameter,
     local, typeDecl, member, literal, identifier, controlStructure,
     cfgNext, callee, callIn, isExternal, reachableByFlows) are the
     most likely to compile and run correctly as-is.

   - Sections marked "BEST EFFORT" use accessors/fields that vary more
     across Joern versions (e.g. cpg.imports, controlStructureType,
     closureBinding, annotation, modifier). They are wrapped in
     Helpers.safe(...) so a RUNTIME failure degrades gracefully — but
     note safe() cannot rescue a COMPILE-time error (an accessor that
     doesn't exist on your build at all). If the script fails to
     compile, the error message will name the exact line/method to fix.

   - The Dominator Graph (Section 11 request) is intentionally left as
     a clearly-marked stub, NOT a guessed implementation. Joern's
     dominator-tree API differs enough across versions that guessing
     the wrong method name here would break compilation of the entire
     script. See the DominatorExtractor object for how to wire it in
     once you confirm the exact API on your build (e.g. via `cpg.help`
     or the interactive REPL with tab-completion).

   - "Reverse call graph" is NOT stored as separate edges — CALLS edges
     are stored one-directionally (caller -> callee). Reverse/RCA-style
     backward traversal is just a direction-reversed Cypher MATCH.
   ===================================================================== */

import scala.collection.mutable
import scala.util.{Try, Success, Failure}
import java.io.{File, PrintWriter}

import io.shiftleft.codepropertygraph.generated.nodes
import io.shiftleft.codepropertygraph.generated.nodes.StoredNode
import io.shiftleft.semanticcpg.language._
import io.joern.dataflowengineoss.language._

// =====================================================================
// CONFIG
// =====================================================================
object Config {
  val outputDir: String = "output"

  val metadataFile     = s"$outputDir/metadata.json"
  val symbolsFile       = s"$outputDir/symbols.json"
  val dependenciesFile  = s"$outputDir/dependencies.json"
  val callgraphFile     = s"$outputDir/callgraph.json"
  val astFile           = s"$outputDir/ast.json"
  val cfgFile           = s"$outputDir/cfg.json"
  val pdgFile           = s"$outputDir/pdg.json"
  val dataflowFile      = s"$outputDir/dataflow.json"
  val metricsFile       = s"$outputDir/metrics.json"
  val summaryFile       = s"$outputDir/summary.json"

  val maxCodeLen: Int = 200

  // Safety caps to keep a POC/large-codebase run bounded in time & size.
  // Raise these once you've measured timing on your real codebase.
  val maxAstNodes: Int              = 200000
  val maxDataFlowSinksPerMethod: Int = 25
  val maxDataFlowMethods: Int        = 2000

  // Interprocedural dataflow (RCA-critical): sources are pulled from the
  // WHOLE cpg (cpg.parameter), not just the current method's own
  // parameters, so a value read in one component (e.g. VehStatus) can be
  // traced forward through several intermediate functions/components
  // (e.g. SWCFUSION -> SWCFEB -> DisplayManager) instead of stopping at
  // the first function boundary. This is what makes multi-hop RCA
  // (calibration/region/TTC -> decision -> display) traceable.
  val maxDataFlowSources: Int        = 1500

  // Global/static variables (module-level state shared across SWCs/tasks)
  // are tagged with isGlobal=true on Variable nodes so agents can find
  // shared-state chains distinct from ordinary local variables.
  val globalScopeMethodNames: Set[String] = Set("<global>")
}

// =====================================================================
// LOGGER
// =====================================================================
object Log {
  def info(msg: String): Unit  = println(s"[INFO]  $msg")
  def warn(msg: String): Unit  = println(s"[WARN]  $msg")
  def error(msg: String): Unit = println(s"[ERROR] $msg")
  def phase(title: String): Unit = println(s"\n===== $title =====")
}

// =====================================================================
// STATISTICS
// =====================================================================
object Statistics {
  val counts = mutable.LinkedHashMap[String, Int]().withDefaultValue(0)
  def inc(key: String, by: Int = 1): Unit = counts(key) = counts(key) + by
  def toJson: String = {
    Json.obj(counts.toList.map { case (k, v) => k -> Json.num(v.toLong) }: _*)
  }
  def print(): Unit = {
    Log.phase("Statistics Summary")
    counts.foreach { case (k, v) => println(f"  $k%-32s $v%10d") }
  }
}

// =====================================================================
// JSON WRITER (dependency-free, manual)
// =====================================================================
object Json {
  def esc(s: String): String = {
    if (s == null) "" else {
      val sb = new StringBuilder
      s.foreach {
        case '"'              => sb.append("\\\"")
        case '\\'             => sb.append("\\\\")
        case '\n'             => sb.append("\\n")
        case '\r'             => sb.append("\\r")
        case '\t'             => sb.append("\\t")
        case c if c.isControl => sb.append(f"\\u${c.toInt}%04x")
        case c                => sb.append(c)
      }
      sb.toString
    }
  }
  def str(v: String): String   = "\"" + esc(v) + "\""
  def num(v: Long): String     = v.toString
  def bool(v: Boolean): String = v.toString
  def obj(fields: (String, String)*): String =
    "{" + fields.map { case (k, v) => s""""$k":$v""" }.mkString(",") + "}"
  def arr(items: Iterable[String]): String =
    "[" + items.mkString(",") + "]"
}

// =====================================================================
// UTILITIES
// =====================================================================
object Utilities {
  def safe[T](default: T)(block: => T): T =
    Try(block) match {
      case Success(v) => v
      case Failure(e) =>
        Log.warn(s"safe() caught: ${e.getClass.getSimpleName}: ${e.getMessage}")
        default
    }

  def lineOf(opt: => Option[Int]): Long =
    safe(0L)(opt.map(_.toLong).getOrElse(0L))

  def ensureOutputDir(): Unit = {
    val dir = new File(Config.outputDir)
    if (!dir.exists()) dir.mkdirs()
  }

  def writeFile(path: String, content: String): Unit = {
    val pw = new PrintWriter(new File(path))
    try pw.write(content) finally pw.close()
  }

  def nid(n: StoredNode): String = n.id().toString
}

// =====================================================================
// GRAPH MODEL — category-scoped so each extractor writes to its own
// JSON export bucket (metadata / symbols / dependencies / callgraph /
// ast / cfg / pdg / dataflow), matching the requested output layout.
// =====================================================================
final case class GNode(
  id: String,
  label: String,
  ntype: String,
  name: String,
  fullName: String,
  code: String,
  file: String,
  line: Long,
  column: Long,
  properties: Map[String, String] = Map.empty
) {
  def toJson: String = {
    val propsJson = Json.obj(properties.toList: _*)
    Json.obj(
      "id"         -> Json.str(id),
      "label"      -> Json.str(label),
      "type"       -> Json.str(ntype),
      "name"       -> Json.str(name),
      "fullName"   -> Json.str(fullName),
      "code"       -> Json.str(code.take(Config.maxCodeLen)),
      "file"       -> Json.str(file),
      "line"       -> Json.num(line),
      "column"     -> Json.num(column),
      "properties" -> propsJson
    )
  }
}

final case class GEdge(
  source: String,
  destination: String,
  relationship: String,
  properties: Map[String, String] = Map.empty
) {
  def toJson: String = Json.obj(
    "source"       -> Json.str(source),
    "destination"  -> Json.str(destination),
    "relationship" -> Json.str(relationship),
    "properties"   -> Json.obj(properties.toList: _*)
  )
}

object GraphModel {
  private val nodesByCategory = mutable.LinkedHashMap[String, mutable.LinkedHashMap[String, GNode]]()
  private val edgesByCategory = mutable.LinkedHashMap[String, mutable.ListBuffer[GEdge]]()
  private val edgeDedup       = mutable.HashSet[(String, String, String, String)]()

  private def nodesFor(cat: String) = nodesByCategory.getOrElseUpdate(cat, mutable.LinkedHashMap.empty)
  private def edgesFor(cat: String) = edgesByCategory.getOrElseUpdate(cat, mutable.ListBuffer.empty)

  def addNode(category: String, n: GNode): Unit = {
    val bucket = nodesFor(category)
    if (!bucket.contains(n.id)) {
      bucket(n.id) = n
      Statistics.inc(s"nodes.$category.${n.label}")
    }
  }

  def addEdge(category: String, src: String, dst: String, rel: String,
              props: Map[String, String] = Map.empty): Unit = {
    val key = (category, src, dst, rel)
    if (!edgeDedup.contains(key)) {
      edgeDedup += key
      edgesFor(category) += GEdge(src, dst, rel, props)
      Statistics.inc(s"edges.$category.$rel")
    }
  }

  def nodesJson(category: String): String = Json.arr(nodesFor(category).values.map(_.toJson))
  def edgesJson(category: String): String = Json.arr(edgesFor(category).map(_.toJson))

  def categoryJson(category: String): String =
    Json.obj("nodes" -> nodesJson(category), "edges" -> edgesJson(category))

  def totalNodes: Int = nodesByCategory.values.map(_.size).sum
  def totalEdges: Int = edgesByCategory.values.map(_.size).sum
}

// =====================================================================
// METADATA EXTRACTOR
//   Project / Directory / File / NamespaceBlock / Module
//   CONTAINS / BELONGS_TO
// =====================================================================
object MetadataExtractor {
  val CATEGORY = "metadata"

  def run(): Unit = {
    Log.phase("MetadataExtractor")
    extractFiles()
    extractNamespaceBlocks()
  }

  private def extractFiles(): Unit = Utilities.safe(())(
    cpg.file.l.foreach { f =>
      val id = Utilities.nid(f)
      GraphModel.addNode(CATEGORY, GNode(
        id       = id,
        label    = "File",
        ntype    = "FILE",
        name     = Utilities.safe("")(f.name),
        fullName = Utilities.safe("")(f.name),
        code     = "",
        file     = Utilities.safe("")(f.name),
        line     = 0L,
        column   = 0L
      ))
    }
  )

  private def extractNamespaceBlocks(): Unit = Utilities.safe(())(
    cpg.namespaceBlock.l.foreach { ns =>
      val id = Utilities.nid(ns)
      val filename = Utilities.safe("")(ns.filename)
      GraphModel.addNode(CATEGORY, GNode(
        id       = id,
        label    = "Module",
        ntype    = "NAMESPACE_BLOCK",
        name     = Utilities.safe("")(ns.name),
        fullName = Utilities.safe("")(ns.fullName),
        code     = "",
        file     = filename,
        line     = Utilities.lineOf(ns.lineNumber),
        column   = 0L
      ))
      // FILE CONTAINS MODULE / MODULE BELONGS_TO FILE
      Utilities.safe(())(
        cpg.file.nameExact(filename).headOption.foreach { f =>
          GraphModel.addEdge(CATEGORY, Utilities.nid(f), id, "CONTAINS")
          GraphModel.addEdge(CATEGORY, id, Utilities.nid(f), "BELONGS_TO")
        }
      )
    }
  )
}

// =====================================================================
// SYMBOL EXTRACTOR
//   Functions/Methods, Parameters, MethodReturn, Locals, TypeDecl,
//   Member, Literal, Identifier
//   HAS_PARAMETER / RETURNS / DECLARES / HAS_MEMBER / USES_TYPE /
//   USES_LITERAL / USES_IDENTIFIER / FILE_OF (via CONTAINS from file)
// =====================================================================
object SymbolExtractor {
  val CATEGORY = "symbols"

  def run(): Unit = {
    Log.phase("SymbolExtractor")
    extractMethods()
    extractParametersAndReturns()
    extractLocals()
    extractTypeDecls()
    extractMembers()
    extractLiterals()
    extractIdentifiers()
  }

  private def fileContainsFunction(fileName: String, methodId: String): Unit =
    Utilities.safe(())(
      cpg.file.nameExact(fileName).headOption.foreach { f =>
        GraphModel.addEdge(CATEGORY, Utilities.nid(f), methodId, "CONTAINS")
        GraphModel.addEdge(CATEGORY, methodId, Utilities.nid(f), "DECLARED_IN")
      }
    )

  private def extractMethods(): Unit = Utilities.safe(())(
    cpg.method.l.foreach { m =>
      val id       = Utilities.nid(m)
      val external = Utilities.safe(false)(m.isExternal)
      val file     = Utilities.safe("")(m.filename)
      GraphModel.addNode(CATEGORY, GNode(
        id       = id,
        label    = "Function",
        ntype    = "METHOD",
        name     = Utilities.safe("")(m.name),
        fullName = Utilities.safe("")(m.fullName),
        code     = Utilities.safe("")(m.code),
        file     = file,
        line     = Utilities.lineOf(m.lineNumber),
        column   = Utilities.lineOf(m.columnNumber),
        properties = Map("isExternal" -> Json.bool(external))
      ))
      fileContainsFunction(file, id)
    }
  )

  private def extractParametersAndReturns(): Unit = {
    Utilities.safe(())(
      cpg.parameter.l.foreach { p =>
        val id = Utilities.nid(p)
        GraphModel.addNode(CATEGORY, GNode(
          id       = id,
          label    = "Parameter",
          ntype    = "METHOD_PARAMETER_IN",
          name     = Utilities.safe("")(p.name),
          fullName = Utilities.safe("")(p.name),
          code     = Utilities.safe("")(p.code),
          file     = Utilities.safe("")(p.method.filename),
          line     = Utilities.lineOf(p.lineNumber),
          column   = 0L
        ))
        Utilities.safe(())(p.method.foreach { m =>
          GraphModel.addEdge(CATEGORY, Utilities.nid(m), id, "HAS_PARAMETER")
          GraphModel.addEdge(CATEGORY, id, Utilities.nid(m), "PARAMETER_OF")
        })
      }
    )

    Utilities.safe(())(
      cpg.methodReturn.l.foreach { r =>
        val id = Utilities.nid(r)
        GraphModel.addNode(CATEGORY, GNode(
          id       = id,
          label    = "MethodReturn",
          ntype    = "METHOD_RETURN",
          name     = "returnType",
          fullName = Utilities.safe("")(r.typeFullName),
          code     = Utilities.safe("")(r.code),
          file     = "",
          line     = Utilities.lineOf(r.lineNumber),
          column   = 0L
        ))
        Utilities.safe(())(r.method.foreach { m =>
          GraphModel.addEdge(CATEGORY, Utilities.nid(m), id, "RETURNS")
        })
      }
    )
  }

  private def extractLocals(): Unit = Utilities.safe(())(
    cpg.local.l.foreach { loc =>
      val id = Utilities.nid(loc)
      val isGlobal = Utilities.safe(false)(
        loc.method.l.exists(m => Config.globalScopeMethodNames.contains(m.name))
      )
      GraphModel.addNode(CATEGORY, GNode(
        id       = id,
        label    = "Variable",
        ntype    = "LOCAL",
        name     = Utilities.safe("")(loc.name),
        fullName = Utilities.safe("")(loc.name),
        code     = Utilities.safe("")(loc.code),
        file     = Utilities.safe("")(loc.method.filename.headOption.getOrElse("")),
        line     = Utilities.lineOf(loc.lineNumber),
        column   = 0L,
        properties = Map(
          "typeFullName" -> Json.str(Utilities.safe("")(loc.typeFullName)),
          "isGlobal"     -> Json.bool(isGlobal)
        )
      ))
      Utilities.safe(())(loc.method.foreach { m =>
        GraphModel.addEdge(CATEGORY, Utilities.nid(m), id, "DECLARES")
        GraphModel.addEdge(CATEGORY, id, Utilities.nid(m), "LOCAL_OF")
        GraphModel.addEdge(CATEGORY, id, Utilities.nid(m), "DECLARED_IN")
      })
    }
  )

  private def extractTypeDecls(): Unit = Utilities.safe(())(
    cpg.typeDecl.l.foreach { t =>
      val id       = Utilities.nid(t)
      val external = Utilities.safe(false)(t.isExternal)
      GraphModel.addNode(CATEGORY, GNode(
        id       = id,
        label    = "Type",
        ntype    = "TYPE_DECL",
        name     = Utilities.safe("")(t.name),
        fullName = Utilities.safe("")(t.fullName),
        code     = "",
        file     = Utilities.safe("")(t.filename),
        line     = Utilities.lineOf(t.lineNumber),
        column   = 0L,
        properties = Map("isExternal" -> Json.bool(external))
      ))
      Utilities.safe(()) {
        val tFile = Utilities.safe("")(t.filename)
        cpg.file.nameExact(tFile).headOption.foreach { f =>
          GraphModel.addEdge(CATEGORY, id, Utilities.nid(f), "DECLARED_IN")
        }
      }
    }
  )

  private def extractMembers(): Unit = Utilities.safe(())(
    cpg.member.l.foreach { mem =>
      val id = Utilities.nid(mem)
      GraphModel.addNode(CATEGORY, GNode(
        id       = id,
        label    = "Member",
        ntype    = "MEMBER",
        name     = Utilities.safe("")(mem.name),
        fullName = Utilities.safe("")(mem.name),
        code     = Utilities.safe("")(mem.code),
        file     = "",
        line     = Utilities.lineOf(mem.lineNumber),
        column   = 0L,
        properties = Map("typeFullName" -> Json.str(Utilities.safe("")(mem.typeFullName)))
      ))
      Utilities.safe(())(mem.typeDecl.foreach { t =>
        GraphModel.addEdge(CATEGORY, Utilities.nid(t), id, "HAS_MEMBER")
        GraphModel.addEdge(CATEGORY, id, Utilities.nid(t), "MEMBER_OF")
      })
    }
  )

  private def extractLiterals(): Unit = Utilities.safe(())(
    cpg.literal.l.foreach { lit =>
      val id = Utilities.nid(lit)
      GraphModel.addNode(CATEGORY, GNode(
        id       = id,
        label    = "Literal",
        ntype    = "LITERAL",
        name     = Utilities.safe("")(lit.code),
        fullName = Utilities.safe("")(lit.code),
        code     = Utilities.safe("")(lit.code),
        file     = Utilities.safe("")(lit.method.filename),
        line     = Utilities.lineOf(lit.lineNumber),
        column   = 0L
      ))
      Utilities.safe(())(lit.method.foreach { m =>
        GraphModel.addEdge(CATEGORY, Utilities.nid(m), id, "USES_LITERAL")
      })
    }
  )

  private def extractIdentifiers(): Unit = Utilities.safe(())(
    cpg.identifier.l.foreach { ident =>
      val id = Utilities.nid(ident)
      GraphModel.addNode(CATEGORY, GNode(
        id       = id,
        label    = "Identifier",
        ntype    = "IDENTIFIER",
        name     = Utilities.safe("")(ident.name),
        fullName = Utilities.safe("")(ident.name),
        code     = Utilities.safe("")(ident.code),
        file     = Utilities.safe("")(ident.method.filename),
        line     = Utilities.lineOf(ident.lineNumber),
        column   = 0L
      ))
      Utilities.safe(())(ident.method.foreach { m =>
        GraphModel.addEdge(CATEGORY, Utilities.nid(m), id, "USES_IDENTIFIER")
      })
      // REFERENCES: identifier -> the local variable it refers to (by name, scoped to its method)
      Utilities.safe(()) {
        val m = ident.method
        cpg.method.fullNameExact(m.fullName).local.nameExact(ident.name).headOption.foreach { l =>
          GraphModel.addEdge(CATEGORY, id, Utilities.nid(l), "REFERENCES")
        }
      }
    }
  )
}

// =====================================================================
// DEPENDENCY EXTRACTOR
//   Calls (forward call graph), Includes/Imports, Reads/Writes/Assigns
//   CALLS / TARGETS / INCLUDES / DEPENDS_ON / READS / WRITES / ASSIGNS
// =====================================================================
object DependencyExtractor {
  val CATEGORY = "dependencies"
  val CALLGRAPH_CATEGORY = "callgraph"

  def run(): Unit = {
    Log.phase("DependencyExtractor")
    extractCalls()
    extractIncludesBestEffort()
    extractAssignments()
  }

  private def extractCalls(): Unit = Utilities.safe(())(
    cpg.call.l.foreach { c =>
      val id = Utilities.nid(c)
      GraphModel.addNode(CALLGRAPH_CATEGORY, GNode(
        id       = id,
        label    = "Call",
        ntype    = "CALL",
        name     = Utilities.safe("")(c.name),
        fullName = Utilities.safe("")(c.methodFullName),
        code     = Utilities.safe("")(c.code),
        file     = Utilities.safe("")(c.method.filename),
        line     = Utilities.lineOf(c.lineNumber),
        column   = 0L
      ))

      Utilities.safe(())(c.method.foreach { m =>
        GraphModel.addEdge(CALLGRAPH_CATEGORY, Utilities.nid(m), id, "CONTAINS")
      })

      // Forward call graph: CALLER -> CALLS -> CALLEE, plus CALL -> TARGETS -> METHOD
      Utilities.safe(())(c.callee.l.foreach { callee =>
        val calleeId = Utilities.nid(callee)
        GraphModel.addEdge(CALLGRAPH_CATEGORY, id, calleeId, "TARGETS")
        Utilities.safe(())(c.method.foreach { caller =>
          GraphModel.addEdge(CALLGRAPH_CATEGORY, Utilities.nid(caller), calleeId, "CALLS")
        })
      })

      // Reads: call arguments that are identifiers/literals -> USES_* on the call
      Utilities.safe(())(c.argument.l.foreach {
        case i: nodes.Identifier =>
          GraphModel.addEdge(CATEGORY, id, Utilities.nid(i), "READS")
        case lit: nodes.Literal =>
          GraphModel.addEdge(CATEGORY, id, Utilities.nid(lit), "USES_LITERAL")
        case _ => ()
      })
    }
  )

  // BEST EFFORT — Import/Include node accessor name and available fields
  // vary across Joern builds/frontends. If `cpg.imports` doesn't exist
  // or returns nothing for a C/C++ CPG, this safely no-ops.
  private def extractIncludesBestEffort(): Unit = Utilities.safe(())(
    cpg.imports.l.foreach { imp =>
      val id = Utilities.nid(imp)
      GraphModel.addNode(CATEGORY, GNode(
        id       = id,
        label    = "Include",
        ntype    = "IMPORT",
        name     = Utilities.safe("")(imp.importedAs.getOrElse("")),
        fullName = Utilities.safe("")(imp.importedEntity.getOrElse("")),
        code     = Utilities.safe("")(imp.code),
        file     = "",
        line     = 0L,
        column   = 0L
      ))
    }
  )

  // Assignment calls: Joern represents `a = b` as a Call node named
  // "<operator>.assignment". We tag WRITES on the LHS identifier and
  // ASSIGNS as the overall relation from the containing method.
  private def extractAssignments(): Unit = Utilities.safe(())(
    cpg.call.nameExact("<operator>.assignment").l.foreach { assign =>
      val id = Utilities.nid(assign)
      Utilities.safe(())(assign.method.foreach { m =>
        GraphModel.addEdge(CATEGORY, Utilities.nid(m), id, "ASSIGNS")
      })
      Utilities.safe(()) {
        val args = assign.argument.l
        args.headOption.foreach {
          case lhs: nodes.Identifier =>
            GraphModel.addEdge(CATEGORY, Utilities.nid(assign), Utilities.nid(lhs), "WRITES")
          case _ => ()
        }
        if (args.size > 1) args.drop(1).foreach {
          case rhs: nodes.Identifier =>
            GraphModel.addEdge(CATEGORY, Utilities.nid(assign), Utilities.nid(rhs), "READS")
          case _ => ()
        }
      }
    }
  )
}

// =====================================================================
// SEMANTIC GRAPH EXTRACTOR — AST / CFG / PDG(approx) / DataFlow
// =====================================================================
object SemanticGraphExtractor {
  val AST_CATEGORY      = "ast"
  val CFG_CATEGORY      = "cfg"
  val PDG_CATEGORY      = "pdg"
  val DATAFLOW_CATEGORY = "dataflow"

  def run(): Unit = {
    Log.phase("SemanticGraphExtractor")
    extractControlStructures()
    extractCfgEdges()
    extractAstEdgesGeneric()
    extractControlDependenceApprox()
    extractConditionDependence()
    extractDataFlowApprox()
  }

  private def extractControlStructures(): Unit = Utilities.safe(())(
    cpg.controlStructure.l.foreach { cs =>
      val id = Utilities.nid(cs)
      GraphModel.addNode(CFG_CATEGORY, GNode(
        id       = id,
        label    = "ControlStructure",
        ntype    = "CONTROL_STRUCTURE",
        name     = Utilities.safe("")(cs.controlStructureType),
        fullName = Utilities.safe("")(cs.controlStructureType),
        code     = Utilities.safe("")(cs.code),
        file     = Utilities.safe("")(cs.method.filename),
        line     = Utilities.lineOf(cs.lineNumber),
        column   = 0L
      ))
      Utilities.safe(())(cs.method.foreach { m =>
        GraphModel.addEdge(CFG_CATEGORY, Utilities.nid(m), id, "CONTAINS")
      })
    }
  )

  // NEXT_CFG edges between call sites and control structures — the two
  // most common CFG node kinds worth tracing for dependency/RCA work.
  private def extractCfgEdges(): Unit = {
    Utilities.safe(())(cpg.call.l.foreach { c =>
      Utilities.safe(())(c.cfgNext.l.foreach { n =>
        GraphModel.addEdge(CFG_CATEGORY, Utilities.nid(c), Utilities.nid(n.asInstanceOf[StoredNode]), "NEXT_CFG")
      })
    })
    Utilities.safe(())(cpg.controlStructure.l.foreach { cs =>
      Utilities.safe(())(cs.cfgNext.l.foreach { n =>
        GraphModel.addEdge(CFG_CATEGORY, Utilities.nid(cs), Utilities.nid(n.asInstanceOf[StoredNode]), "NEXT_CFG")
      })
    })
  }

  // Generic AST_CHILD edges over the already-extracted node set (methods,
  // parameters, locals, calls, control structures, literals, identifiers,
  // members). We deliberately scope to nodes we've already modeled rather
  // than walking cpg.all's full raw AST (blocks, every sub-expression,
  // etc.), which would balloon output size without adding much value for
  // dependency/RCA/RAG use cases. Adjust `Config.maxAstNodes` if you want
  // a raw full-AST dump instead.
  private def extractAstEdgesGeneric(): Unit = Utilities.safe(()) {
    var count = 0
    def link(parent: StoredNode, child: StoredNode): Unit = {
      if (count < Config.maxAstNodes) {
        GraphModel.addEdge(AST_CATEGORY, Utilities.nid(parent), Utilities.nid(child), "AST_CHILD")
        count += 1
      }
    }
    Utilities.safe(())(cpg.method.l.foreach { m =>
      Utilities.safe(())(m.parameter.l.foreach(p => link(m, p)))
      Utilities.safe(())(m.local.l.foreach(l => link(m, l)))
      Utilities.safe(())(m.call.l.foreach(c => link(m, c)))
      Utilities.safe(())(m.controlStructure.l.foreach(cs => link(m, cs)))
    })
    Utilities.safe(())(cpg.typeDecl.l.foreach { t =>
      Utilities.safe(())(t.member.l.foreach(mem => link(t, mem)))
    })
    Log.info(s"AST_CHILD edges generated: $count")
  }

  // PDG (control-dependence approximation): every call/identifier that is
  // a structural descendant of a control structure is treated as control
  // dependent on it. This uses the generic, stable `.ast` traversal step
  // rather than guessing at Joern's internal PDG edge API.
  private def extractControlDependenceApprox(): Unit = Utilities.safe(())(
    cpg.controlStructure.l.foreach { cs =>
      val csId = Utilities.nid(cs)
      Utilities.safe(())(cs.ast.isCall.l.foreach { c =>
        GraphModel.addEdge(PDG_CATEGORY, csId, Utilities.nid(c), "CONTROL_DEPENDENCE")
      })
      Utilities.safe(())(cs.ast.isIdentifier.l.foreach { i =>
        GraphModel.addEdge(PDG_CATEGORY, csId, Utilities.nid(i), "CONTROL_DEPENDENCE")
      })
    }
  )

  // Data-flow / data-dependence: INTERPROCEDURAL, scoped and capped so it
  // can't hang on a large CPG.
  //
  // IMPORTANT: sources are drawn from cpg.parameter across the WHOLE cpg,
  // not m.parameter (the sink's own method only). Joern's dataflow engine
  // (reachableByFlows) is natively interprocedural -- it will walk through
  // intermediate calls, returns, and assignments across function/component
  // boundaries as long as the source set isn't artificially restricted to
  // "this same function". The earlier per-method restriction was the bug:
  // it made every flow stop at the first function boundary, which silently
  // hides exactly the multi-hop chains RCA needs (e.g. a calibration value
  // read in one SWC, passed through 2-3 intermediate functions, and used
  // in a decision several components later).
  private def extractDataFlowApprox(): Unit = Utilities.safe(()) {
    val allSources = Utilities.safe(List.empty[nodes.MethodParameterIn])(
      cpg.parameter.l.take(Config.maxDataFlowSources)
    )
    val methods = Utilities.safe(List.empty[nodes.Method])(
      cpg.method.filterNot(_.isExternal).l.take(Config.maxDataFlowMethods)
    )
    methods.foreach { m =>
      val sinks = Utilities.safe(List.empty[nodes.Call])(
        m.call.l.take(Config.maxDataFlowSinksPerMethod)
      )
      sinks.foreach { sink =>
        Utilities.safe(())(
          sink.argument.reachableByFlows(allSources).l.foreach { flow =>
            val elems = flow.elements
            elems.sliding(2).foreach {
              case Seq(a: StoredNode, b: StoredNode) =>
                GraphModel.addEdge(DATAFLOW_CATEGORY, Utilities.nid(a), Utilities.nid(b), "DATA_FLOW")
                GraphModel.addEdge(PDG_CATEGORY, Utilities.nid(a), Utilities.nid(b), "DATA_DEPENDENCE")
              case _ => ()
            }
          }
        )
      }
    }
  }

  // Condition-linked control dependence: links a control structure
  // (if/while/for) not just to what's INSIDE its body (already handled by
  // extractControlDependenceApprox above), but to the specific
  // identifiers/calls used in its CONDITION expression itself. This is
  // what lets an agent answer "which exact variable gated this branch"
  // (e.g. region_variant, ttc_threshold, ignition_state) instead of only
  // knowing "this code is inside some if-block".
  private def extractConditionDependence(): Unit = Utilities.safe(())(
    cpg.controlStructure.l.foreach { cs =>
      val csId = Utilities.nid(cs)
      Utilities.safe(())(cs.condition.l.foreach { cond =>
        Utilities.safe(())(cond.ast.isIdentifier.l.foreach { i =>
          GraphModel.addEdge(PDG_CATEGORY, csId, Utilities.nid(i), "GATED_BY")
        })
        Utilities.safe(())(cond.ast.isCall.l.foreach { c =>
          GraphModel.addEdge(PDG_CATEGORY, csId, Utilities.nid(c), "GATED_BY")
        })
      })
    }
  )
}

// =====================================================================
// DOMINATOR EXTRACTOR — INTENTIONAL STUB
//
// Joern's dominator-tree accessor name/shape has varied across releases
// enough that guessing it here risks a COMPILE-time failure across the
// ENTIRE script (safe() only catches runtime errors). Rather than risk
// that, this is left unimplemented on purpose.
//
// To wire it in once you confirm the API on 4.0.583:
//   1. Open the interactive shell:  joern
//   2. importCpg("../ADAS/cpg.bin")
//   3. Try tab-completion on a CFG node, e.g.:
//        cpg.method.head.cfgNode.head.<TAB>
//      looking for something like `.dominatedBy` / `.dominates` /
//      `.postDominates` (exact name varies by version).
//   4. Once confirmed, add a DominatorExtractor.run() call from Main
//      below, following the same GraphModel.addEdge(...) pattern used
//      in extractControlDependenceApprox() above, with relationship
//      types "DOMINATES" / "POST_DOMINATES".
// =====================================================================
object DominatorExtractor {
  def run(): Unit = {
    Log.phase("DominatorExtractor (STUB — not implemented, see comments above)")
    Log.warn("Dominator/post-dominator extraction is not implemented — see DominatorExtractor comment block.")
  }
}

// =====================================================================
// METRIC EXTRACTOR
//   LOC, Cyclomatic Complexity, Fan-In/Out, Parameter/Return Count,
//   Function Count, Variable Count, AST/CFG/PDG size, Dependency Count
// =====================================================================
object MetricExtractor {

  final case class MethodMetric(
    name: String, fullName: String, file: String,
    loc: Long, complexity: Int, fanIn: Int, fanOut: Int,
    paramCount: Int, returnCount: Int, isExternal: Boolean
  ) {
    def toJson: String = Json.obj(
      "name"        -> Json.str(name),
      "fullName"    -> Json.str(fullName),
      "file"        -> Json.str(file),
      "loc"         -> Json.num(loc),
      "complexity"  -> Json.num(complexity.toLong),
      "fanIn"       -> Json.num(fanIn.toLong),
      "fanOut"      -> Json.num(fanOut.toLong),
      "paramCount"  -> Json.num(paramCount.toLong),
      "returnCount" -> Json.num(returnCount.toLong),
      "isExternal"  -> Json.bool(isExternal)
    )
  }

  var perMethod: List[MethodMetric] = Nil

  def run(): Unit = {
    Log.phase("MetricExtractor")
    perMethod = Utilities.safe(List.empty[MethodMetric])(
      cpg.method.filterNot(_.name.startsWith("<")).l.map { m =>
        val startLine = Utilities.lineOf(m.lineNumber)
        val endLine   = Utilities.lineOf(m.lineNumberEnd)
        val loc       = if (endLine >= startLine) (endLine - startLine + 1) else 0L
        MethodMetric(
          name        = Utilities.safe("")(m.name),
          fullName    = Utilities.safe("")(m.fullName),
          file        = Utilities.safe("")(m.filename),
          loc         = loc,
          complexity  = Utilities.safe(1)(m.controlStructure.size + 1),
          fanIn       = Utilities.safe(0)(m.callIn.size),
          fanOut      = Utilities.safe(0)(m.call.size),
          paramCount  = Utilities.safe(0)(m.parameter.size),
          returnCount = Utilities.safe(0)(m.methodReturn.size),
          isExternal  = Utilities.safe(false)(m.isExternal)
        )
      }
    )
    Statistics.inc("metrics.methodsProfiled", perMethod.length)
  }

  def toJson: String = Json.obj(
    "perMethod"     -> Json.arr(perMethod.map(_.toJson)),
    "totalNodes"    -> Json.num(GraphModel.totalNodes.toLong),
    "totalEdges"    -> Json.num(GraphModel.totalEdges.toLong),
    "functionCount" -> Json.num(perMethod.length.toLong)
  )
}

// =====================================================================
// NEO4J EXPORTER
//   Writes one JSON file per category, matching the requested layout:
//   metadata.json, symbols.json, dependencies.json, callgraph.json,
//   ast.json, cfg.json, pdg.json, dataflow.json, metrics.json,
//   summary.json
// =====================================================================
object Neo4jExporter {
  def exportAll(): Unit = {
    Log.phase("Neo4jExporter")
    Utilities.ensureOutputDir()

    Utilities.writeFile(Config.metadataFile,    GraphModel.categoryJson("metadata"))
    Utilities.writeFile(Config.symbolsFile,      GraphModel.categoryJson("symbols"))
    Utilities.writeFile(Config.dependenciesFile, GraphModel.categoryJson("dependencies"))
    Utilities.writeFile(Config.callgraphFile,    GraphModel.categoryJson("callgraph"))
    Utilities.writeFile(Config.astFile,          GraphModel.categoryJson("ast"))
    Utilities.writeFile(Config.cfgFile,          GraphModel.categoryJson("cfg"))
    Utilities.writeFile(Config.pdgFile,          GraphModel.categoryJson("pdg"))
    Utilities.writeFile(Config.dataflowFile,     GraphModel.categoryJson("dataflow"))
    Utilities.writeFile(Config.metricsFile,      MetricExtractor.toJson)
    Utilities.writeFile(Config.summaryFile,      Statistics.toJson)

    Log.info(s"metadata.json     -> ${Config.metadataFile}")
    Log.info(s"symbols.json      -> ${Config.symbolsFile}")
    Log.info(s"dependencies.json -> ${Config.dependenciesFile}")
    Log.info(s"callgraph.json    -> ${Config.callgraphFile}")
    Log.info(s"ast.json          -> ${Config.astFile}")
    Log.info(s"cfg.json          -> ${Config.cfgFile}")
    Log.info(s"pdg.json          -> ${Config.pdgFile}")
    Log.info(s"dataflow.json     -> ${Config.dataflowFile}")
    Log.info(s"metrics.json      -> ${Config.metricsFile}")
    Log.info(s"summary.json      -> ${Config.summaryFile}")
  }
}

// =====================================================================
// MAIN
// =====================================================================
Log.phase("GSCEF — Generic Semantic CPG Extraction Framework")
Utilities.ensureOutputDir()
Log.info(s"Output directory: ${Config.outputDir}")

Utilities.safe(())(MetadataExtractor.run())
Utilities.safe(())(SymbolExtractor.run())
Utilities.safe(())(DependencyExtractor.run())
Utilities.safe(())(SemanticGraphExtractor.run())
Utilities.safe(())(DominatorExtractor.run())
Utilities.safe(())(MetricExtractor.run())

Neo4jExporter.exportAll()

Statistics.print()

println(s"\nTotal nodes: ${GraphModel.totalNodes}")
println(s"Total edges: ${GraphModel.totalEdges}")
println("\nDONE.")