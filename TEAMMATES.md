# AIOS — Team & Collaboration Tracker

> Living collaboration document. Update this file whenever responsibilities, progress, dependencies, handoffs, or important decisions change.

---

# 1. Project

**Project:** AIOS — AI-Powered Multiprogramming Operating System Simulator

**Goal:** Build an integrated C++ operating-system simulator with simulated AI-agent workloads and a React visualization dashboard.

---

# 2. Team

| Member | Role | Primary Responsibility | Status |
|---|---|---|---|
| Manthan Gadiya | Project member | Core OS Implementation (M1-M6) | Active |
| TBD | Team member | TBD | Unknown |
| TBD | Team member | TBD | Unknown |
| TBD | Team member | TBD | Unknown |
| TBD | Team member | TBD | Unknown |

> Do not invent team assignments. Replace `TBD` only when confirmed by the team.

---

# 3. Responsibility Areas

The project naturally divides into:

### Core OS

- CPU / Machine Simulator
- Process Management
- Memory / Paging
- Interrupts / System Calls
- Scheduling
- Synchronization
- IPC
- I/O / Buffering / Spooling
- Error Handling

### AI Layer

- AI Agent Manager
- AI Adaptive Scheduler
- Resource Quotas
- Permissions
- Agent IPC / Knowledge Bus
- Checkpointing
- Failure / Recovery

### Frontend

- React Dashboard
- CPU visualization
- Process visualization
- Memory visualization
- Scheduler visualization
- IPC visualization
- AI-agent visualization
- Event logs
- Metrics
- Scenario controls

### Integration

- C++ ↔ frontend communication
- REST / WebSocket
- Scenario engine
- End-to-end testing
- Demo reliability

---

# 4. Current Allocation

Assignments must be confirmed before being treated as official.

| Area | Owner | Backup | Status |
|---|---|---|---|
| CPU | Manthan Gadiya | TBD | 🟢 Complete |
| Process Management | Manthan Gadiya | TBD | 🟢 Complete |
| Memory / Paging | Manthan Gadiya | TBD | 🟡 In Progress (flat RAM done, paging next) |
| Interrupts / System Calls | Manthan Gadiya | TBD | 🟢 Complete (stub I/O/IPC) |
| Scheduling | TBD | TBD | ⚪ Not Started |
| Synchronization | TBD | TBD | ⚪ Not Started |
| IPC | TBD | TBD | ⚪ Not Started |
| I/O | TBD | TBD | ⚪ Not Started |
| AI Agent Layer | TBD | TBD | ⚪ Not Started |
| GUI | TBD | TBD | ⚪ Not Started |
| Integration | TBD | TBD | ⚪ Not Started |
| Testing | Manthan Gadiya | TBD | 🟢 Unit tests passing |

---

# 5. Confirmed Project Decisions

### Architecture

C++ is the simulation source of truth.

React is the presentation/control layer.

### AI

AI agents are simulated workloads.

They are represented as OS-managed processes.

### Scheduling

The AI scheduler uses explainable heuristics rather than requiring a real machine-learning model.

### Demonstration

The final demo uses controlled and repeatable scenarios.

### Scope

Real hardware, real LLM control, real autonomous agents, distributed OS functionality and production-level security are outside the initial scope.

---

# 6. Collaboration Rules

Before modifying another subsystem:

1. Read its documentation.
2. Inspect its code.
3. Understand its public interface.
4. Avoid changing unrelated interfaces.
5. Communicate breaking changes.
6. Update this file if ownership/dependencies change.

---

# 7. Handoff Format

When handing work to another teammate:

```text
Subsystem:
Owner:
Completed:
Current State:
Files:
Public Interfaces:
Tests:
Known Issues:
Dependencies:
Next Recommended Step:
```

---

# 8. Change Log

## 2026-08-19

* M1+M2 Core OS Implementation complete.
* CPU Simulator: registers, fetch-decode-execute, full instruction set, FLAGS, error detection.
* Memory: 1024-word flat RAM, program loading, bounds checking.
* Process Manager: PCB, state machine, queues, dispatch/preemption with context save/restore.
* Interrupt Manager: SYSTEM_CALL, TIMER, IO_COMPLETE, ERROR, pending queue + log.
* System Call Manager: READ, WRITE, ALLOCATE, EXIT (stubs for IPC, I/O).
* Event Log, Simulation Clock, Program Loader.
* All unit tests passing (9 test files).
* CMake + MinGW build verified.
* GitHub repository confirmed and synced (https://github.com/ManthanGadiya/AIOS.git).

## 2026-08-18

* Project-level PRD direction established.
* Agent workflow requirements defined.
* Living project status tracking requested.
* Team collaboration tracking requested.
* Requirement established to keep README updates limited to Current Status / Quick Start.
* Existing documentation reviewed.
* No GitHub repository could be identified yet; repository/commit workflow is therefore pending repository access.
* STATUS.md and TEAMMATES.md populated from the copies embedded in AGENTS.md.
* Canonical filenames confirmed as plural: AGENT.md → AGENTS.md and 14_Demo_Scenario.md → 14_Demo_Scenarios.md; references updated across README.md, AGENTS.md, docs/PRD.md and docs/01_Project_Overview.md.
* Canonical repository structure established: backend/, frontend/ and tests/ directories created; PRD and ROADMAP structure trees updated to match the actual repository.

---

# 9. Open Collaboration Questions

1. What are the confirmed names of all five team members?
2. What subsystem is assigned to each member?
3. Which GitHub repository contains AIOS? → **Answered: https://github.com/ManthanGadiya/AIOS.git**
4. Which branch is the team's primary development branch? → **Answered: main**

---

# 10. Handoff Principle

Every teammate should be able to open:

```text
TEAMMATES.md
STATUS.md
PRD.md
```

and understand:

```text
Who is doing what?
What has been completed?
What is currently happening?
What is blocked?
What should I work on next?
```