# ADAS Dependency Tracing POC

A realistic, AUTOSAR-inspired ADAS (Advanced Driver Assistance System) software repository built to evaluate AI-assisted dependency tracing using **Joern**, **Neo4j**, and **LangGraph**.

The project simulates an OEM automotive software repository in which engineers investigate why a system output does not reach its expected state. Rather than debugging software defects, the goal is to trace the dependencies — functions, configurations, calibrations, state machines, calculations, and supplier boundaries — that influence a given output.

---

## Project Objective

Modern ADAS software consists of thousands of interconnected functions distributed across multiple software components (SWCs). When an unexpected behavior appears on the HMI or ECU output, engineers often spend hours manually navigating source code to understand how that output was derived.

This repository provides a realistic C codebase enabling automated dependency tracing through static analysis and graph-based reasoning, demonstrating how Large Language Models (LLMs), Code Property Graphs (CPGs), and Knowledge Graphs can assist engineers during software investigations.

---

## Project Goals

- Build a realistic OEM ADAS software repository
- Generate a Code Property Graph (CPG) using Joern
- Export the graph into Neo4j
- Perform dependency tracing via graph traversal
- Use LangGraph agents to automate engineering investigations
- Explain why a software output does or doesn't reach a desired state

---

## Technology Stack

| Technology | Purpose |
|---|---|
| **C** | AUTOSAR-inspired ADAS software implementation |
| **Joern** | Static code analysis and CPG generation |
| **Neo4j** | Storage and traversal of function, variable, and dependency graphs |
| **LangGraph** | Multi-agent orchestration for investigation workflows |
| **Python** | Graph extraction, orchestration, and automation |

---

## System Architecture

```
                 CAN Messages
                      │
                      ▼
                  CAN Interface
                      │
                      ▼
                 Vehicle Status
                      │
                      ▼
                      RTE
        ┌─────────────┼─────────────┐
        ▼             ▼             ▼
     Camera         Radar        Supplier
        │             │             │
        └──────┬──────┘             │
               ▼                    │
            Fusion                  │
               │                    │
               ▼                    │
              FEB <─────────────────┘
               │
               ▼
         Vehicle Status Out
               │
               ▼
              RTE
               │
               ▼
              HMI
               │
               ▼
        Display Manager
```

---

## Repository Structure

```
ADAS
│
├── OEMSWC
│   ├── Application
│   ├── CANIF
│   ├── Common
│   ├── Configuration
│   ├── Diagnostics
│   ├── DisplayManager
│   ├── RTE
│   ├── SWCCAM
│   ├── SWCFEB
│   ├── SWCFUSION
│   ├── SWCHMI
│   ├── SWCRAD
│   └── VehStatus
│
└── SUPSWC
    └── SWCFEB_SUP
```

---

## Module Description

| Module | Description |
|---|---|
| **Application** | Entry point, scheduler, and execution order of software components |
| **CANIF** | Simulates CAN message reception and transmission |
| **Common** | Shared types, constants, utilities, and helper functions |
| **Configuration** | Feature flags, calibration values, specifications, and vehicle variants |
| **Diagnostics** | Health monitoring, timeout handling, and diagnostic information |
| **DisplayManager** | Updates cluster and HMI display outputs |
| **RTE** | Runtime Environment APIs (`Rte_Read()` / `Rte_Write()`) for SWC communication |
| **VehStatus** | Central storage of vehicle signals exchanged between software components |
| **SWCCAM** | Camera processing and object validation |
| **SWCRAD** | Radar processing and target validation |
| **SWCFUSION** | Camera-radar sensor fusion and threat calculations |
| **SWCFEB** | Forward Emergency Braking decision logic and state machine |
| **SWCHMI** | Reads vehicle outputs and updates HMI information |
| **SUPSWC** | Supplier-owned functionality, accessible only through interface APIs |

---

## AI Investigation Pipeline

```
C Source Code
       │
       ▼
     Joern
(Code Property Graph)
       │
       ▼
 Graph Export
       │
       ▼
    Neo4j
       │
       ▼
Dependency Queries
       │
       ▼
 LangGraph Agents
       │
       ▼
Engineering Investigation Report
```

---

## LangGraph Agents

| Agent | Responsibility |
|---|---|
| **Discovery Agent** | Traverses the dependency graph to identify all functions, variables, and modules influencing the requested output |
| **Reasoning Agent** | Analyzes the discovered dependency chain to explain software behavior |
| **Evidence Agent** | Collects supporting code snippets, variables, constants, and function relationships from the graph |
| **Report Agent** | Generates a structured engineering investigation report with traceability evidence |

---

## Role of Joern

Joern parses the C source code and generates a **Code Property Graph (CPG)** combining:

- Abstract Syntax Tree (AST)
- Control Flow Graph (CFG)
- Data Flow Graph (DFG)
- Call Graph

This enables tracing of function calls, variable usage, data flow, control flow, and dependencies across software components.

---

## Role of Neo4j

Neo4j stores the extracted dependency graph and enables efficient traversal of relationships such as:

- Function → Function
- Function → Variable
- Variable → Signal
- Signal → Module
- Module → Configuration
- Configuration → Calibration
- State Machine → Output

This supports dependency tracing across hundreds or thousands of interconnected functions.

---

## Role of LangGraph

LangGraph orchestrates multiple reasoning agents that collaborate during software investigations. Instead of searching files sequentially, agents operate directly on the dependency graph to produce explainable engineering reasoning.

---

## Investigation Scenarios

| Investigation | User Symptom | Expected Output | Actual Output | Investigation Scope | Final Dependency |
|---|---|---|---|---|---|
| **INV-01** | FEB warning icon not displayed on HMI | `FEB_WarningStatus = WARNING_REQUESTED` | `FEB_WarningStatus = OPERATIONAL` | Camera → Fusion → FEB → Configuration → Display | OEM Configuration, Calibration, Region Variant |
| **INV-02** | Automatic brake request remains inactive | `FEB_BrakeRequest = ACTIVE` | `FEB_BrakeRequest = INACTIVE` | HMI → RTE → Supplier Interface | Supplier-owned implementation (trace boundary) |
| **INV-03** | FEB system remains NOT_OPERATIONAL | `FEB_SystemState = OPERATIONAL` | `FEB_SystemState = NOT_OPERATIONAL` | HMI → FEB State Machine → Configuration → Feature Enable Logic | Vehicle Variant, Feature Flags, Ignition State, Calibration |

---

## Expected Investigation Flow

An engineer starts with a software output, for example:

> Why is FEB Warning not displayed?

The system then:

1. Identifies the output signal
2. Discovers all software dependencies
3. Traverses the complete call graph
4. Identifies influencing variables and configurations
5. Stops at OEM or supplier boundaries
6. Generates an explainable engineering investigation report

---

## Repository Characteristics

- AUTOSAR-inspired architecture
- OEM-focused repository
- Supplier boundary simulation
- Deep function call hierarchy
- Multiple software layers
- Configuration-driven behavior
- Calibration-dependent logic
- State machine implementation
- Mathematical calculations
- RTE-based communication
- Rich dependency graph suitable for static analysis

---

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