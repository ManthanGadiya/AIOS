

# AIOS Agent Development Instructions

## 1. Purpose

This document defines how any AI coding/development agent must work on the AIOS project.

AIOS is an educational C++-based Multiprogramming Operating System Simulator with a simulated AI-agent workload layer and a React-based visualization dashboard.

The agent must treat the repository documentation as the primary source of truth.

---

# 2. Project Definition

AIOS is:

> A C++-based Multiprogramming Operating System Simulator that implements CPU and machine simulation, supervisor calls, interrupts, process management, paging, virtual memory, scheduling, synchronization, IPC, I/O, buffering, and spooling, and extends this foundation with simulated AI-agent workloads, adaptive scheduling, resource management, permissions, agent communication, checkpointing, and failure recovery.

The frontend visualizes and controls the simulator.

The frontend is NOT the operating system.

The C++ simulation engine is the source of truth.

---

# 3. Core Architectural Rule

Always preserve:

```text
User
 ↓
React Dashboard
 ↓
REST / WebSocket
 ↓
C++ Simulation Engine
 ↓
OS Components
 ↓
Simulated Hardware
````

AI agents operate inside the OS:

```text
AI Agent
 ↓
AI Agent Manager
 ↓
Process Manager
 ↓
Scheduler
 ↓
CPU
```

The AI layer must not bypass core OS mechanisms.

---

# Agent Subagents
- while planning some feature, use planner subagent
- while thinking about some topic, use a research subagent
- while coding some logic, use a coder subagent
- while reviewing some code, use a reviewer subagent
- while designing some frontend or need creativity, use a designer subagent
---

# Agent Workflow
- first research
- then planner
- then designer or coder
- then reviewer

``` Make sure to use the right subagent for the job.```

``` You have to use them all.```

``` And you have to use them in the right order.```

``` And you have to use them frequently.```

# 4. Source of Truth Hierarchy

When making a decision, inspect sources in this order:

1. Existing implementation
2. `PRD.md`
3. `STATUS.md`
4. `TEAMMATES.md`
5. `docs/03_System_Architecture.md`
6. Relevant subsystem documentation
7. Tests
8. Other repository documentation
9. External research only when necessary

Do not silently contradict existing project decisions.

If documentation conflicts with implementation:

1. Identify the conflict.
2. Do not arbitrarily choose one.
3. Explain the conflict.
4. Ask the project owner if the intended behaviour cannot be inferred safely.

---

# 5. Mandatory Workflow

Every meaningful task follows:

```text
INSPECT
  ↓
UNDERSTAND
  ↓
PLAN
  ↓
IMPLEMENT
  ↓
TEST
  ↓
DOCUMENT
  ↓
UPDATE STATUS
  ↓
UPDATE TEAMMATES
  ↓
UPDATE README CURRENT STATUS
  ↓
COMMIT
```

Never skip inspection.

Never implement first and investigate later.

---

# 6. Documentation-First Rule

Before implementing a subsystem:

```text
Search docs/
      ↓
Find existing specification
      ↓
Read relevant sections
      ↓
Inspect related code
      ↓
Implement according to specification
```

Relevant documents include:

```text
docs/
├── 01_Project_Overview.md
├── 02_Requirements.md
├── 03_System_Architecture.md
├── 04_CPU_Machine_Simulation.md
├── 05_System_Calls_Interrupts.md
├── 06_Process_Management.md
├── 07_Paging_Virtual_Memory.md
├── 08_Scheduling.md
├── 09_Synchronization.md
├── 10_IPC.md
├── 11_IO_Buffering_Spooling.md
├── 12_AI_Agent_Layer.md
├── 13_GUI_Architecture.md
├── 14_Demo_Scenarios.md
└── 15_Testing.md
└── PRD.md
└── ROADMAP.md
```

---

# 7. Missing Information Rule

If the required information cannot be found in:

* project documentation
* existing code
* tests
* configuration
* project history

then STOP and ask the user.

Do NOT invent:

* architecture
* API contracts
* team assignments
* repository structure
* requirements
* technology decisions
* behaviour
* scheduling formulas
* demo requirements

Example:

> The documentation does not specify whether failed-agent IPC messages should be retained or discarded. Which behaviour should we use?

---

# 8. Skill Usage

Before starting a substantial task, inspect available skills.

Use the project's skill-discovery mechanism when available.

In particular, use:

```text
cavemen skill
ponytail skill
find-skill
```

Use `find-skill` to locate the optimal available skill for the current task.

Do not blindly install or invoke unrelated skills.

Choose skills according to the task:

```text
Architecture → architecture/design skill
C++ → C++ engineering skill
React → frontend skill
Testing → testing skill
Documentation → documentation skill
Git → Git/GitHub workflow skill
```

If the required skill does not exist, continue using sound engineering practices rather than fabricating a skill.

---

# 9. MCP / Tool Usage

Use available MCPs and connected tools when they materially improve the task.

Potential tools include:

* Agent Memory
* Firecrawl
* MarkItDown
* Reticle
* Ruflo

Use them when appropriate.

Examples:

```text
Agent Memory
→ preserve useful project decisions/context

Firecrawl
→ external documentation/research when required


MarkItDown
→ convert/inspect supported documents

Reticle
→ repository/code analysis when available

Ruflo
→ orchestration/workflow tasks when appropriate
```

Do not use tools merely because they exist.

Prefer the simplest reliable tool for the task.

---

# 10. Ongoing Project Loop

The agent must continuously behave like a senior project developer.

After every meaningful task:

```text
What changed?
What is now working?
What remains?
What is blocked?
What decision was made?
What should happen next?
```

Then update:

```text
STATUS.md
TEAMMATES.md
README.md
```

Only the designated Current Status section of `README.md` should be maintained by this workflow.

---

# 11. README Rule

Do not rewrite the entire README.

Only maintain:

```md
## Current Status
```

and its associated:

```md
### Quick Start
```

content.

Do not modify unrelated README sections unless the user explicitly asks.

---

# 12. Status Rule

`STATUS.md` is a living project state document.

It must contain:

* Current phase
* Overall progress
* Completed work
* Active work
* Blocked work
* Known issues
* Decisions
* Next actions
* Latest meaningful change
* Latest commit
* Last updated timestamp

Never claim something is complete unless there is evidence.

---

# 13. Team Rule

`TEAMMATES.md` is the collaboration/handoff document.

It must track:

* Team members
* Responsibilities
* Current assignments
* Completed work
* Active work
* Dependencies
* Handoffs
* Decisions
* Unassigned work

If a team member identity or assignment is unknown:

```text
UNKNOWN
```

Do not invent names.

---

# 14. Commit Policy

Commit after every meaningful, coherent change.

Good commits:

```text
docs: add AIOS agent workflow
docs: add project status tracking
feat(cpu): implement instruction execution
feat(process): add PCB lifecycle
feat(memory): implement page table
feat(scheduler): add round robin policy
feat(ai): add simulated agent manager
feat(gui): add process dashboard
test(memory): add page fault tests
fix(ipc): correct blocked receiver wakeup
```

Avoid:

```text
update
changes
final
done
stuff
```

A commit should explain the intent.

---

# 15. Commit Granularity

Prefer:

```text
One logical change
      ↓
One coherent commit
```

Avoid mixing unrelated changes:

```text
CPU + GUI + README + scheduler
```

in one commit unless they are genuinely one atomic change.

---

# 16. Testing Policy

Before declaring a feature complete:

```text
Compile
 ↓
Unit tests
 ↓
Integration tests
 ↓
Relevant scenario
 ↓
Full regression when appropriate
```

Never mark a feature complete based only on compilation.

---

# 17. AIOS-Specific Rules

AI agents must:

* Have an OS process representation.
* Have a PID.
* Have process state.
* Use OS scheduling.
* Use simulated memory.
* Use OS IPC.
* Respect resource quotas.
* Respect permissions.
* Support checkpointing.
* Support failure/recovery where implemented.

The AI layer must not directly control simulated CPU hardware.

---

# 18. Determinism

Core demonstrations should be deterministic wherever practical.

Avoid introducing:

* unnecessary randomness
* external API dependencies
* real-time dependencies
* uncontrolled AI model behaviour

into the core demo path.

---

# 19. Scope Protection

Do not add:

* Real LLM scheduling
* Real hardware control
* Real autonomous agents
* GPU scheduling
* Distributed OS features
* Production security
* Real hardware drivers

unless the project owner explicitly changes the scope.

---

# 20. Communication Rule

When blocked, ask a focused question.

Bad:

> What should I do?

Good:

> The current documentation specifies both REST and WebSocket communication but does not define which endpoint owns simulation state. Should the backend expose REST for commands and WebSocket for state streaming?

---

# 21. Senior Developer Standard

Before completing work, ask:

* Does this follow the architecture?
* Does this introduce unnecessary coupling?
* Is the behaviour testable?
* Is the behaviour deterministic?
* Is the API clear?
* Is the documentation updated?
* Can another teammate understand the change?
* Can the professor observe the feature during the demo?
* Did this accidentally increase project scope?

---

# 22. Final Rule

AIOS development is not:

```text
Write Code → Hope It Works
```

It is:

```text
Understand
 → Design
 → Implement
 → Verify
 → Document
 → Communicate
 → Commit
 → Continue
```

````

---

# 👥 `TEAMMATES.md`

```md
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
| Manthan Gadiya | Project member | TBD | Active |
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
| CPU | TBD | TBD | Not Started |
| Process Management | TBD | TBD | Not Started |
| Memory / Paging | TBD | TBD | Not Started |
| Interrupts / System Calls | TBD | TBD | Not Started |
| Scheduling | TBD | TBD | Not Started |
| Synchronization | TBD | TBD | Not Started |
| IPC | TBD | TBD | Not Started |
| I/O | TBD | TBD | Not Started |
| AI Agent Layer | TBD | TBD | Not Started |
| GUI | TBD | TBD | Not Started |
| Integration | TBD | TBD | Not Started |
| Testing | TBD | TBD | Not Started |

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
````

---

# 8. Change Log

## 2026-08-18

* Project-level PRD direction established.
* Agent workflow requirements defined.
* Living project status tracking requested.
* Team collaboration tracking requested.
* Requirement established to keep README updates limited to Current Status / Quick Start.
* Existing documentation reviewed.
* No GitHub repository could be identified yet; repository/commit workflow is therefore pending repository access.

---

# 9. Open Collaboration Questions

1. What are the confirmed names of all five team members?
2. What subsystem is assigned to each member?
3. Which GitHub repository contains AIOS?
4. Which branch is the team's primary development branch?

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

````

---

# 📊 `STATUS.md`

```md
# AIOS — Project Status

> Living project-status document.
>
> Update this file after every meaningful project change.

**Last Updated:** 2026-08-18

---

# 1. Current Phase

## Phase: Project Definition / Documentation

Current priority:

```text
Project Definition
      ↓
Documentation Governance
      ↓
Repository Setup / Verification
      ↓
Core OS Implementation
      ↓
AI Layer
      ↓
GUI
      ↓
Integration
      ↓
Testing
      ↓
Final Demo
````

---

# 2. Overall Status

**Status:** 🟡 Planning / Documentation

**Implementation Status:** Not yet verified from repository

**GitHub Status:** Repository not currently identified

**Current Focus:**

* Establish project documentation governance.
* Establish agent workflow.
* Establish team tracking.
* Establish living project status.
* Verify repository before code changes.

---

# 3. What Has Been Defined

The project specification currently defines:

### Core OS

* CPU / Machine Simulation
* Process Management
* Multiprogramming
* Scheduling
* Paging
* Virtual Memory
* Page Faults
* Interrupts
* System Calls
* Synchronization
* IPC
* I/O
* Buffering
* Spooling
* Error Handling

### AI Layer

* Simulated AI agents
* AI agents represented as OS processes
* AI adaptive scheduling
* Explainable scheduling decisions
* Resource quotas
* Permissions
* Agent communication
* Checkpointing
* Failure
* Recovery

### GUI

* React
* TypeScript
* Tailwind CSS
* Dashboard
* CPU visualization
* Process visualization
* Memory visualization
* Scheduler visualization
* IPC visualization
* AI-agent visualization
* Event logs
* Metrics

### Communication

* REST / WebSocket
* JSON

---

# 4. Current Architecture

```text
User
 ↓
React Dashboard
 ↓
REST / WebSocket
 ↓
C++ AIOS Engine
 ↓
┌───────────────────────────┐
│ CPU                       │
│ Process Manager           │
│ Scheduler                 │
│ Memory Manager            │
│ Interrupt Manager         │
│ System Call Manager       │
│ Synchronization Manager   │
│ IPC Manager               │
│ I/O Manager               │
│ Error Manager             │
└──────────────┬────────────┘
               ↓
        AI Agent Manager
               ↓
        AI Agent Workloads
```

The C++ engine remains the simulation source of truth.

---

# 5. Documentation State

| Document                      | State           |
|-------------------------------| --------------- |
| PRD.md                        | Defined         |
| 01 Project Overview           | Existing        |
| 02 Requirements               | Existing        |
| 03 Architecture               | Existing        |
| 04 CPU                        | Existing        |
| 05 System Calls / Interrupts  | Existing        |
| 06 Process Management         | Existing        |
| 07 Paging / Virtual Memory    | Existing        |
| 08 Scheduling                 | Existing        |
| 09 Synchronization            | Existing        |
| 10 IPC                        | Existing        |
| 11 I/O / Buffering / Spooling | Existing        |
| 12 AI Agent Layer             | Existing        |
| 13 GUI Architecture           | Existing        |
| 14 Demo Scenarios             | Existing        |
| 15 Testing                    | Existing        |
| AGENTS.md                     | To be committed |
| TEAMMATES.md                  | To be committed |
| STATUS.md                     | To be committed |

---

# 6. Completed Planning Work

### Completed

* High-level AIOS concept defined.
* Problem statement defined.
* Core OS subsystem list defined.
* AI-agent model defined.
* AI scheduling concept defined.
* AI resource-management concept defined.
* Checkpoint/recovery concept defined.
* GUI architecture direction defined.
* Technology stack defined.
* Demo flow defined.
* Project scope boundaries defined.
* Documentation structure defined.
* PRD direction established.
* Agent workflow requirements established.
* Team tracking requirements established.
* Status tracking requirements established.

---

# 7. Currently In Progress

### Documentation Governance

Establish:

```text
AGENTS.md
TEAMMATES.md
STATUS.md
PRD.md
```

and define how they remain synchronized during development.

### Repository Verification

The GitHub repository has not yet been identified through the currently available GitHub integration.

No source-code implementation should be assumed to exist until the repository is confirmed.

---

# 8. Not Yet Verified

The following require repository inspection:

* Actual source-code structure
* Actual C++ implementation
* Actual frontend implementation
* Existing tests
* Build system
* Current branches
* Existing commits
* GitHub repository
* Current implementation progress
* Team member assignments

---

# 9. Current Blockers

## BLOCKER-001 — GitHub Repository

The repository identity is currently unavailable.

Required:

```text
GitHub repository:
owner/repository
```

Without this, repository files cannot safely be modified or committed.

---

## BLOCKER-002 — Team Assignments

The project documentation does not establish the final five-member ownership map.

Required:

```text
Member → Responsibility
```

Do not invent assignments.

---

# 10. Next Actions

### Immediate

1. Confirm GitHub repository.
2. Inspect repository structure.
3. Inspect current source code.
4. Inspect current branches.
5. Inspect recent commits.
6. Compare implementation against documentation.
7. Confirm team assignments.
8. Commit project governance documents.
9. Establish implementation baseline.
10. Start first Core OS subsystem.

---

# 11. Recommended Implementation Order

```text
CPU
 ↓
Process Management
 ↓
Memory / Paging
 ↓
Interrupts / System Calls
 ↓
Scheduling
 ↓
Synchronization
 ↓
IPC
 ↓
I/O / Buffering / Spooling
 ↓
AI Agent Layer
 ↓
REST / WebSocket
 ↓
React Dashboard
 ↓
Integration
 ↓
Testing
```

---

# 12. Definition of Done

A subsystem is not considered complete until:

```text
Specification understood
        ↓
Implementation completed
        ↓
Unit tests pass
        ↓
Integration tested
        ↓
Relevant demo scenario works
        ↓
Documentation updated
        ↓
STATUS updated
        ↓
TEAMMATES updated
        ↓
README Current Status updated
        ↓
Meaningful commit created
```

---

# 13. Latest Meaningful Change

**2026-08-18**

Governance documents populated and documentation references corrected.

* STATUS.md and TEAMMATES.md were populated from the copies embedded in AGENTS.md.
* Canonical documentation filenames confirmed as plural: `AGENT.md` → `AGENTS.md` and `14_Demo_Scenario.md` → `14_Demo_Scenarios.md`.
* Filename references corrected across README.md, AGENTS.md, docs/PRD.md and docs/01_Project_Overview.md.
* Canonical repository structure established: `backend/`, `frontend/` and `tests/` directories created; structure trees in docs/PRD.md and docs/ROADMAP.md aligned with the actual repository.
* GitHub repository and commit workflow remain pending until the repository is identified.

---

# 14. Latest Commit

```text
5ba3d88 docs: establish repository structure and governance baseline
```

---

# 15. Current Status for README

The README should contain only a concise version of this state:

```text
AIOS is currently in the project-definition/documentation phase.

The architecture, OS subsystems, AI-agent layer, GUI direction,
demo scenarios and testing strategy have been defined.

Next:
1. Verify repository
2. Establish implementation baseline
3. Begin CPU / Machine Simulator
4. Implement core OS incrementally
```

---

# 16. Status Legend

```text
🟢 Complete
🟡 In Progress
🔵 Planned
🔴 Blocked
⚪ Not Started
```

---

# 17. Important Rule

Never use this document to make the project appear more complete than it is.

Status must reflect evidence from:

```text
Code
Tests
Commits
Documentation
Demonstrations
```

not intentions.

