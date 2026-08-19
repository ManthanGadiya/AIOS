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
```

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
- while planning some feature, use the planner subagent
- while thinking about some topic, use the research subagent
- while coding some logic, use the coder subagent
- while reviewing some code, use the reviewer subagent
- while designing some frontend or need creativity, use the designer subagent

---

# Agent Workflow
- first research
- then planner
- then designer or coder
- then reviewer

> Make sure to use the right subagent for the job.
> You have to use them all, in the right order, and frequently.

---

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
├── PRD.md
├── ROADMAP.md
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

Use the project's skill-discovery mechanism when available (for example `find-skill`) to locate the optimal available skill for the current task.

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

Connected tools may include (depending on the agent environment):

* Agent Memory
* Firecrawl
* MarkItDown
* GitHub

Use them when appropriate.

Examples:

```text
Agent Memory
→ preserve useful project decisions/context

Firecrawl
→ external documentation/research when required

MarkItDown
→ convert/inspect supported documents

GitHub
→ repository inspection, issues and pull requests
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

---

# 23. Living Documents

`STATUS.md`, `TEAMMATES.md`, `docs/PRD.md` and `docs/ROADMAP.md` are living documents maintained directly in their own files.

Their content is intentionally not duplicated here.

```text
STATUS.md       ← living project status
TEAMMATES.md    ← team and collaboration tracker
docs/PRD.md     ← product requirements and scope
docs/ROADMAP.md ← phased implementation plan
```