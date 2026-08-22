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
| Process Management | Manthan Gadiya | TBD | 🟢 Complete (PCB, states, queues, context switch, resource tracking, statistics) |
| Memory / Paging | Manthan Gadiya | TBD | 🟢 Complete (flat RAM + page tables, frames, swap, FIFO) |
| Interrupts / System Calls | Manthan Gadiya | TBD | 🟢 Complete (priority queue, timer, nested policy, PAGE_FAULT dispatch; stub I/O/IPC) |
| Scheduling | TBD | TBD | ⚪ Not Started |
| Synchronization | TBD | TBD | ⚪ Not Started |
| IPC | TBD | TBD | ⚪ Not Started |
| I/O | TBD | TBD | ⚪ Not Started |
| AI Agent Layer | TBD | TBD | ⚪ Not Started |
| GUI | TBD | TBD | ⚪ Not Started |
| Integration | TBD | TBD | ⚪ Not Started |
| Testing | Manthan Gadiya | TBD | 🟢 Unit tests passing (10 test files) |

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

## 2026-08-22

* Mid-review GUI fix pass after first live testing (build layout, port guard, real stats, UI fit).
* Second UI polish pass from live review feedback: CPU panel compressed to a one-row register strip with inline flags; physical memory as a 16x4 chip grid with usage counter; page table now toggles between processes with prev/next buttons instead of stacking all tables.
* New Scheduler panel (docs/13 sections 27, 29): policy/on-CPU/switch summary plus a Gantt-style CPU timeline rendered from real schedulingHistory segments and recent decision reasons - gives FCFS and Priority visible progress between their (intentionally rare) context switches; replaces the empty IPC placeholder slot until Week 6.
* Canonical build directory is backend/build; stray repository-root build removed. Quick Start unchanged.
* Server now probes the port before binding and exits with code 1 when an instance already runs there - fixes "frontend showed stale data after I stopped the backend" (stale instance was still serving).
* Dashboard statistics are real: CPU utilization from ProcessManager::cpuUtilization(), memory usage from MemoryManager counters, page faults counted, scheduler metrics averaged from process statistics.
* UI shows Engine Connected/Disconnected honestly, with a banner when the engine is unreachable; controls only reflect accepted commands.
* All dashboard panels have fixed heights with internal scrollbars so the full system view fits on screen (docs/13 sections 59-60); event log category filters now functional.
* Full suite still 131 test cases / 717 assertions passing.

* Mid-review GUI complete: REST server + React dashboard (roadmap section 9 "Mid Review GUI" scope).
* C++ REST API server (aios/server, cpp-httplib + nlohmann/json in third_party): /api/statistics, /api/processes, /api/agents, /api/cpu, /api/memory, /api/scheduler, /api/interrupts, /api/events, POST /api/command.
* START/PAUSE/STOP gate a 10 cycles/s host simulation loop; RESET restores the initial workload; CHANGE_SCHEDULER maps onto SchedulingPolicy.
* Standard demo workload per PRD section 20: P1 TextEditor, P2 Compiler, P3 MusicPlayer + A1 Research Agent, A2 Coding Agent via createAgentProcess(); long-running programs so scheduling is visibly live during the demo.
* Fixed CPU wiring: interrupt manager was not attached, so demand-paging page faults failed processes; now fault -> frame load -> instruction retry works live. HALT reaps TERMINATED, CPU-error halts reap FAILED.
* React 19 + TypeScript + Tailwind v4 dashboard: TopBar controls with live cycle indicator, sidebar navigation, stat cards, process table, CPU registers panel, physical frame grid, page tables, ready queue, filterable event log, interrupt panel. All values fetched from the REST API (docs/13 sections 7-30, 59).
* Verified end-to-end on port 8081 (frontend dev on 5173): RR context switches cycling five workloads, page-fault handling visible in event log, pause freezes cycle, reset restores initial state. Full suite still 131 test cases / 717 assertions passing.
* Committed and pushed to https://github.com/ManthanGadiya/AIOS.git.

## 2026-08-20

* M5 Scheduler complete: FCFS, Round Robin and Priority policies.
* SchedulingPolicy (FCFS / ROUND_ROBIN / PRIORITY); larger priority value = higher priority, preemptive (docs/08 section 13).
* Persistent ready order reconciled with the Process Manager's READY set on every decision (docs/08 section 6).
* FCFS non-preemptive FIFO; Round Robin preempts on quantum expiry and rotates the running process to the back (docs/08 sections 7-10); Priority preempts only on strictly higher priority.
* Scheduler events: SCHEDULER_STARTED, PROCESS_SELECTED, PROCESS_PREEMPTED, TIME_QUANTUM_EXPIRED (docs/08 section 40); decision history + context-switch count + reset().
* TIMER interrupt wired through InterruptManager::setScheduler() so Round Robin runs from the interrupt path (docs/08 section 24).
* Unit tests: test_scheduler.cpp (16 T-SCHED cases); full suite 131 test cases / 717 assertions passing.
* Committed and pushed to https://github.com/ManthanGadiya/AIOS.git.

* M5 Process Manager full features complete (creation data, resource tracking, statistics).
* ProcessType (NORMAL / AI_AGENT) on the PCB; createAgentProcess() wrapper; AI agents remain ordinary OS processes (docs/06 section 34).
* CPU-time accounting charged on CPU release (preempt/block/terminate/fail/reset); waiting-time accounting charged centrally when leaving the READY queue.
* Response time (first run), turnaround time (terminated - created), state-change and context-switch counters; ProcessStatistics API (getProcessStatistics / getAllProcessStatistics / cpuUtilization).
* Per-process page faults surfaced into process statistics from the Memory Manager counters.
* Unit tests: T-PROC-014..023 + T-PAGE-015; full suite 115 test cases / 611 assertions passing.
* Committed and pushed to https://github.com/ManthanGadiya/AIOS.git.

* M4 Full Interrupt Manager complete.
* Interrupt priority queue: pending interrupts serviced in priority order (ERROR > PAGE_FAULT > SYSTEM_CALL > IO_COMPLETE > TIMER), FIFO within equal priority; ordering centralized in InterruptManager::priorityOf().
* Nested-interrupt policy: interrupts arriving while an ISR executes are deferred, never nested; serviceNextInterrupt() refuses reentrancy.
* Timer component: periodic TIMER interrupts on a configurable quantum (default 4 cycles); basis for Round Robin scheduling.
* Lifecycle observability (SERVICING -> COMPLETED) and pendingInterrupts() accessor.
* Unit tests: test_timer.cpp (7 T-TIMER cases) + T-INT-009/010/011; T-INT-002 updated from FIFO to priority; full suite 104 test cases / 524 assertions passing.
* docs/05 section 19 updated with the finalized priority ordering.
* Committed and pushed to https://github.com/ManthanGadiya/AIOS.git.

* M3 Paging / Virtual Memory complete.
* Memory Manager: page tables, frame table, simulated swap, demand paging, FIFO page replacement with dirty write-back.
* PAGE_FAULT interrupt type added; Interrupt Manager dispatches faults to the Memory Manager.
* CPU integrated: paged fetch/execute with instruction retry; invalid access fails the process via ERROR interrupt.
* Process Manager integrated: paged createProcess with legacy flat-Memory fallback; memory released on terminate/fail/reset.
* Memory statistics and 8 new memory event types added.
* Unit tests: test_paging.cpp (14 T-PAGE cases); full suite 94 test cases / 442 assertions passing.
* Committed and pushed to https://github.com/ManthanGadiya/AIOS.git.

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