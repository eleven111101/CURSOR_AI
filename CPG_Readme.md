# Code Property Graph (CPG) using Joern
### ADAS Static Code Analysis Platform — Team Onboarding & Reference Guide

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [What is Static Code Analysis?](#2-what-is-static-code-analysis)
3. [What is Joern?](#3-what-is-joern)
4. [What is a Code Property Graph (CPG)?](#4-what-is-a-code-property-graph-cpg)
5. [Why We Use CPG for ADAS](#5-why-we-use-cpg-for-adas)
6. [Platform Architecture](#6-platform-architecture)
7. [Software & System Requirements](#7-software--system-requirements)
8. [Installing WSL](#8-installing-wsl)
9. [Installing Ubuntu](#9-installing-ubuntu)
10. [Setting Up VS Code with WSL](#10-setting-up-vs-code-with-wsl)
11. [Installing Java](#11-installing-java)
12. [Installing Joern](#12-installing-joern)
13. [Verifying the Installation](#13-verifying-the-installation)
14. [Project Folder Structure](#14-project-folder-structure)
15. [Parsing Source Code](#15-parsing-source-code)
16. [Generated Files Explained](#16-generated-files-explained)
17. [Querying the CPG](#17-querying-the-cpg)
18. [Exporting Graphs](#18-exporting-graphs)
19. [Neo4j Integration (Planned)](#19-neo4j-integration-planned)
20. [AI + RAG Integration (Planned)](#20-ai--rag-integration-planned)
21. [Troubleshooting](#21-troubleshooting)
22. [FAQ](#22-faq)
23. [Appendix: Command Cheat Sheet](#23-appendix-command-cheat-sheet)

---

## 1. Introduction

This document is the central onboarding and reference guide for the **CPG (Code Property Graph) analysis environment**, built using **Joern**, for the ADAS (Advanced Driver Assistance Systems) project.

The purpose of this platform is to build a graph-based representation of our source code that can be used for:

- Static code analysis
- Vulnerability and defect detection
- Dependency and call-graph analysis
- Data-flow tracing across modules
- AI-based code understanding
- Retrieval-Augmented Generation (RAG) pipelines for code Q&A
- Graph visualization via Neo4j

Anyone joining the project — whether working on parsing, queries, graph visualization, or the AI layer — should be able to go from zero to fully set up using this document alone.

---

## 2. What is Static Code Analysis?

Static code analysis means examining source code **without executing it**, to find structural issues, security flaws, or architectural patterns.

```
Source Code
     ↓
  Parser
     ↓
Graph Representation
     ↓
Security / Structural Analysis
     ↓
   Reports
```

Unlike runtime debugging or dynamic testing, static analysis:

- Doesn't require the program to build or run
- Can analyze incomplete or partial codebases
- Finds issues early, before code reaches test or production
- Scales well across large codebases (like an ADAS stack)

---

## 3. What is Joern?

**Joern** is an open-source static code analysis platform purpose-built for source code security analysis. It parses source code into a **Code Property Graph** and exposes a powerful Scala-based query language to traverse it.

**Supported languages include:**

- C / C++
- Java / Kotlin
- Python
- JavaScript / TypeScript
- Go
- PHP
- LLVM IR

**Official site:** https://joern.io

For our ADAS project, Joern is primarily used against C/C++ modules, though the same pipeline extends to other languages in the stack as needed.

---

## 4. What is a Code Property Graph (CPG)?

A CPG merges several classical program representations into a single unified graph:

- **AST** (Abstract Syntax Tree) — the syntactic structure of the code
- **CFG** (Control Flow Graph) — the order in which statements can execute
- **PDG** (Program Dependence Graph) — data and control dependencies
- **Call Graph** — which functions call which
- **Data Flow Graph** — how values propagate through the program

```
   AST
    +
   CFG
    +
   PDG
    +
 Data Flow
    +
Call Graph
    ↓
   CPG
```

Because all of these are merged into one graph, a single query can answer questions that would otherwise require combining multiple separate tools (e.g., "which user-controlled inputs eventually reach this braking function, and through which call path?").

---

## 5. Why We Use CPG for ADAS

ADAS pipelines are deeply interconnected — a single sensor input can flow through many modules before it affects a physical actuator:

```
Camera
   ↓
Object Detection
   ↓
Planning
   ↓
Control
   ↓
Braking
```

With a CPG, we can:

- Trace the complete execution and data-flow path from sensor input to actuator output
- Identify unsafe or unchecked data reaching safety-critical functions
- Detect architectural drift between modules over time
- Provide a queryable knowledge base for both engineers and downstream AI tooling

This is far more powerful than isolated linting or single-file static analysis, especially for a safety-critical system like ADAS.

---

## 6. Platform Architecture

End-to-end pipeline, from raw source to AI-assisted querying:

```
ADAS Source Code
       ↓
  Joern Parse
       ↓
     CPG (cpg.bin)
       ↓
  Joern Queries
       ↓
   JSON / GraphML Export
       ↓
      Neo4j  (graph storage + visualization)
       ↓
  Embeddings + RAG
       ↓
       LLM  (natural-language code Q&A)
```

Today, the team primarily works within the **Joern Parse → CPG → Queries** stages. The **Neo4j** and **RAG/LLM** stages are planned next steps and are documented in Sections 19 and 20 for forward planning.

---

## 7. Software & System Requirements

| Component | Version / Notes |
|---|---|
| Windows | 10 (2004+) or 11 |
| WSL | Version 2 |
| Ubuntu (in WSL) | 22.04 or 24.04 LTS |
| Java (JDK) | 17 (Temurin/OpenJDK recommended) |
| VS Code | Latest, with "WSL" extension |
| Git | Latest |
| Joern | 4.x (latest stable release) |
| Disk space | 10+ GB free recommended for CPGs on large codebases |
| RAM | 8 GB minimum, 16 GB+ recommended for large parses |

---

## 8. Installing WSL

Run these in an **elevated (Administrator) PowerShell** window on Windows:

```powershell
wsl --install
```

This installs WSL2 and a default Ubuntu distribution in one step. Restart Windows if prompted.

Check WSL status:

```powershell
wsl --status
```

List installed distributions and confirm the WSL version in use:

```powershell
wsl -l -v
```

You should see `Ubuntu` listed with **VERSION 2**. If it shows version 1, upgrade it:

```powershell
wsl --set-version Ubuntu 2
```

---

## 9. Installing Ubuntu

If Ubuntu wasn't installed automatically, install it explicitly from the Microsoft Store, or via:

```powershell
wsl --install -d Ubuntu-24.04
```

On first launch, Ubuntu will ask you to create a **Linux username and password** — this is separate from your Windows login and is used for `sudo` commands inside WSL.

Once inside Ubuntu, update the package index and installed packages:

```bash
sudo apt update
sudo apt upgrade -y
```

Install common build tools that Joern and its dependencies rely on:

```bash
sudo apt install -y build-essential curl wget git unzip
```

---

## 10. Setting Up VS Code with WSL

VS Code can edit and run everything directly inside your Ubuntu (WSL) filesystem, which avoids cross-filesystem path and permission issues.

Steps:

1. Install **VS Code** on Windows (not inside WSL).
2. Install the **"WSL" extension** (`ms-vscode-remote.remote-wsl`) from the Extensions marketplace.
3. Open a WSL terminal and navigate to your project folder, then run:

```bash
code .
```

This reopens VS Code connected to the Ubuntu environment — the terminal, file explorer, and extensions all now operate inside Linux rather than Windows.

```
Normal VS Code (Windows)
        ↓
 Install "WSL" Extension
        ↓
  Open Folder in WSL
        ↓
   Ubuntu Terminal in VS Code
```

**Why this matters:** Joern and its scripts expect a Linux environment. Editing files on the Windows filesystem (`C:` or `D:`) and running Joern against them from WSL causes slow I/O and occasional path/permission errors — so keep the project inside the Linux filesystem (e.g., `/home/<user>/...`).

---

## 11. Installing Java

**Why Java?** Joern is written in Scala, which compiles to and runs on the **JVM**. Java is therefore a hard requirement:

```
Scala (Joern's implementation language)
        ↓
       JVM
        ↓
      Java (runtime)
```

Install OpenJDK 17 inside Ubuntu (WSL):

```bash
sudo apt install -y openjdk-17-jdk
```

Verify:

```bash
java -version
javac -version
```

Confirm `JAVA_HOME` is set (add to `~/.bashrc` if missing):

```bash
echo 'export JAVA_HOME=$(dirname $(dirname $(readlink -f $(which javac))))' >> ~/.bashrc
source ~/.bashrc
echo $JAVA_HOME
```

---

## 12. Installing Joern

Download the official Joern installer:

```bash
wget https://github.com/joernio/joern/releases/latest/download/joern-install.sh
```

Make it executable:

```bash
chmod +x joern-install.sh
```

Run the installer:

```bash
./joern-install.sh
```

**What each step does:**

| Command | Purpose |
|---|---|
| `wget ...` | Downloads the Joern installation script from GitHub Releases |
| `chmod +x joern-install.sh` | Grants execute permission to the downloaded script |
| `./joern-install.sh` | Downloads Joern's binaries/dependencies and installs the `joern`, `joern-parse`, and related CLI tools |

By default, Joern installs to a directory under your home folder, e.g.:

```
/home/<user>/bin/joern/joern-cli
```

Add Joern's CLI to your `PATH` if the installer hasn't already (check your `~/.bashrc`):

```bash
export PATH="$HOME/bin/joern/joern-cli:$PATH"
source ~/.bashrc
```

---

## 13. Verifying the Installation

Run:

```bash
joern --version
```

Launch the interactive Joern shell to confirm it starts correctly:

```bash
joern
```

You should see a Scala/Joern REPL prompt (`joern>`). Exit with:

```
:quit
```

If any of these fail, see [Section 21: Troubleshooting](#21-troubleshooting).

---

## 14. Project Folder Structure

**Windows side (source of truth for editing, if applicable):**

```
D:\
└── RAG_11
    └── cursor ai
```

**Linux side (WSL — where Joern actually runs):**

```
/home/<user>
└── bin
    └── joern
        └── joern-cli
```

**Why these differ:** Windows and WSL use separate filesystems. Windows paths (`D:\...`) are mounted into WSL under `/mnt/d/...`, but running heavy parsing tools like Joern against `/mnt/d/...` is significantly slower and more error-prone than running them against native Linux paths (`/home/<user>/...`). The recommended workflow is:

1. Keep the canonical source under version control (Git).
2. Clone/copy the repository into the Linux filesystem (`/home/<user>/projects/...`) before parsing.
3. Use VS Code's WSL mode (Section 10) to edit those same Linux-side files directly — no manual syncing required.

---

## 15. Parsing Source Code

From inside your project directory (on the **Linux filesystem**):

```bash
joern-parse .
```

**What happens internally:**

```
Source Code
     ↓
   Parser
     ↓
    AST
     ↓
    CFG
     ↓
    PDG
     ↓
    CPG
     ↓
  cpg.bin
```

`joern-parse` walks the project directory, parses each recognized source file, builds the AST/CFG/PDG/call-graph layers, merges them into a single Code Property Graph, and serializes the result to a binary file: **`cpg.bin`**.

Depending on codebase size, this can take anywhere from seconds to tens of minutes — large ADAS modules should be parsed on a machine with sufficient RAM (see Section 7).

---

## 16. Generated Files Explained

| File / Folder | Description |
|---|---|
| `cpg.bin` | The serialized Code Property Graph — the core artifact used by all subsequent Joern queries |
| `ast/` | Optional exported Abstract Syntax Tree data (if explicitly exported) |
| `cfg/` | Optional exported Control Flow Graph data |
| `pdg/` | Optional exported Program Dependence Graph data |
| `graphml/` | GraphML export of the CPG — the format consumed by Neo4j and other graph visualization tools |
| `dot/` | Graphviz DOT-format export — useful for lightweight local visualization |

In day-to-day use, `cpg.bin` is the only file most engineers interact with directly; the others are generated on demand when exporting for visualization (Section 18) or Neo4j ingestion (Section 19).

---

## 17. Querying the CPG

Open the CPG in the interactive shell:

```bash
joern
joern> importCpg("cpg.bin")
```

Example queries (Joern's query language is Scala-based and uses the CPGQL DSL):

```scala
// List all method names
cpg.method.name.l

// List all function/method calls
cpg.call.name.l

// List all local variables
cpg.local.name.l

// List all type/class declarations
cpg.typeDecl.name.l

// Find all calls to a specific function
cpg.call.name("applyBrake").l

// Trace where a variable's value flows to
cpg.identifier("sensorInput").reachableBy(cpg.parameter).l
```

These traversals are the foundation for the vulnerability/data-flow analyses the team will build on top of the CPG.

---

## 18. Exporting Graphs

Export the full CPG to GraphML (for Neo4j) or DOT (for Graphviz):

```bash
joern-export cpg.bin --repr all --out output_graphml --format graphml
joern-export cpg.bin --repr all --out output_dot --format dot
```

- Use **GraphML** exports when the destination is Neo4j (Section 19).
- Use **DOT** exports for quick local visualization with Graphviz or VS Code DOT extensions.

---

## 19. Neo4j Integration (Planned)

```
     CPG (cpg.bin)
          ↓
   GraphML Export
          ↓
        Neo4j
          ↓
    Visualization / Cypher Queries
```

Once the GraphML export is generated (Section 18), it can be imported into a Neo4j instance for:

- Interactive graph visualization
- Cypher-based querying, complementary to Joern's own DSL
- Long-term persistent storage of parsed CPGs across builds

This integration is planned but not yet implemented for the team pipeline — this section will be expanded with concrete import steps once the Neo4j environment is finalized.

---

## 20. AI + RAG Integration (Planned)

```
ADAS Source Code
       ↓
     Joern
       ↓
      CPG
       ↓
   JSON Export
       ↓
   Embeddings
       ↓
     FAISS  (vector search)
       ↓
     Neo4j  (graph context)
       ↓
      LLM  (natural-language Q&A over the codebase)
```

The long-term goal is to let engineers ask natural-language questions about the ADAS codebase (e.g., *"What functions can influence the braking output?"*) and have the system answer using a combination of:

- Vector similarity search (FAISS) over code embeddings
- Structural/graph context pulled from Neo4j
- An LLM that synthesizes both into a grounded answer

This section will be filled in with implementation details as this part of the platform is built out.

---

## 21. Troubleshooting

| Problem | Likely Cause | Solution |
|---|---|---|
| `java: command not found` | Java not installed or not on `PATH` | Reinstall via `sudo apt install openjdk-17-jdk`; confirm with `java -version` |
| WSL not installed / `wsl` not recognized | WSL feature not enabled | Run `wsl --install` in an elevated PowerShell, then restart Windows |
| Ubuntu not installed / no distros listed | Distro install step skipped or failed | Run `wsl --install -d Ubuntu-24.04`; check with `wsl -l -v` |
| `Permission denied` running scripts | Script lacks execute permission | Run `chmod +x <script>.sh` before executing it |
| Joern installed but not found in new terminals | `PATH` update not persisted | Re-add the export line to `~/.bashrc` (not just the current shell) and run `source ~/.bashrc` |
| `joern` command not found | Installation didn't complete, or `PATH` missing the `joern-cli` directory | Re-run `./joern-install.sh`; verify `joern-cli` is on `PATH` |
| `joern-parse` not found | Same as above — partial install or `PATH` issue | Confirm the full `joern-cli` toolset is present under the install directory |
| `cpg.bin` missing after parse | Parse failed silently, or wrong working directory | Re-run `joern-parse .` from inside the actual project root; check for parse errors in the console output |
| Very slow parsing | Running against `/mnt/d/...` (Windows-mounted path) instead of native Linux path | Copy/clone the project into `/home/<user>/...` before parsing |

---

## 22. FAQ

**Why WSL instead of native Windows?**
Joern's toolchain and installer scripts are built and tested for Linux. WSL2 gives a genuine Linux kernel and filesystem on Windows, avoiding compatibility issues with native Windows execution.

**Why Java?**
Joern is implemented in Scala, which runs on the JVM. Java is a direct runtime dependency — there's no way to run Joern without it.

**Why can't I see Joern on the `D:` drive?**
Joern is installed inside the WSL (Linux) filesystem, at a path like `/home/<user>/bin/joern`, which is a separate filesystem from Windows drives. It's not visible under `D:\` because it was never installed there.

**Can I install Joern directly on Windows?**
Joern doesn't officially support a native Windows installation path. WSL2 is the supported and recommended approach for Windows users.

**What is a CPG, in one line?**
A single graph that merges a program's syntax tree, control flow, data dependencies, and call graph into one structure that can be queried together.

**What's the difference between AST and CPG?**
An AST only captures syntactic structure (what the code looks like). A CPG adds control flow, data flow, and call relationships on top of the AST, enabling much richer queries (e.g., tracing how a value moves through the program).

**Can I use Neo4j today?**
Not yet as part of the standard team pipeline — Neo4j integration is planned (Section 19). GraphML exports can already be generated manually if you want to experiment locally.

---

## 23. Appendix: Command Cheat Sheet

```bash
# --- WSL / Windows (PowerShell) ---
wsl --install
wsl --status
wsl -l -v
wsl --set-version Ubuntu 2

# --- Ubuntu setup ---
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential curl wget git unzip

# --- Java ---
sudo apt install -y openjdk-17-jdk
java -version

# --- Joern install ---
wget https://github.com/joernio/joern/releases/latest/download/joern-install.sh
chmod +x joern-install.sh
./joern-install.sh
joern --version

# --- Parsing ---
joern-parse .

# --- Interactive queries ---
joern
joern> importCpg("cpg.bin")
joern> cpg.method.name.l

# --- Exporting ---
joern-export cpg.bin --repr all --out output_graphml --format graphml
joern-export cpg.bin --repr all --out output_dot --format dot
```

## Disclaimer

This repository is a research-oriented software project developed to evaluate AI-assisted dependency tracing techniques for automotive software engineering. The implementation is inspired by common AUTOSAR software architectures but does not represent production vehicle software.

## Citation

```
ADAS Dependency Tracing POC
GRAVITY ~ AI, 2026
```

---

## Author

**GRAVITY ~ AI** — ADAS Dependency Tracing POC, v1.0