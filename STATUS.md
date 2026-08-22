# AIOS — Project Status

> Living project-status document.
>
> Update this file after every meaningful project change.

**Last Updated:** 2026-08-20

---

# 1. Current Phase

## Phase: Core OS Implementation

Current priority:

```text
Project Definition
      ↓
Documentation Governance
      ↓
Repository Setup / Verification
      ↓
Core OS Implementation  ← CURRENT
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
```

---

# 2. Overall Status

**Status:** 🟢 Core OS Implementation — M1/M2/M3 + M4 (Full Interrupt Manager) + M5 (Process Manager Full Features) Complete

**Implementation Status:** M1+M2+M3 + M4 + M5 Complete — CPU, Memory, Process Management (full features), Interrupts, System Calls, Paging/Virtual Memory, Interrupt Priority/Timer, Process Resource Tracking/Statistics implemented and tested

**GitHub Status:** https://github.com/ManthanGadiya/AIOS.git (main)

**Current Focus:**

* Core OS implementation (M1: Foundation + M2: Stage I CPU/System Calls + M3: Paging + M4: Full Interrupt Manager + M5: Process Manager full features) — **DONE**
* C++ backend builds and tests pass (115 test cases / 611 assertions)
* GitHub repository confirmed and synced
* Next: Scheduling (Week 5)

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
| AGENTS.md                     | Committed       |
| TEAMMATES.md                  | Committed       |
| STATUS.md                     | Committed       |

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

### Implemented (M1+M2)

* **CPU Simulator** — registers (PC, IR, ACC, MAR, MBR, FLAGS), fetch-decode-execute, instruction set (LOAD, STORE, ADD, SUB, MUL, DIV, JMP, JZ, READ, WRITE, SYSCALL, HALT), arithmetic flags, error detection
* **Memory** — 1024-word flat RAM, program loading, bounds checking
* **Process Manager** — PCB, state machine (NEW/READY/RUNNING/WAITING/TERMINATED/FAILED), ready/waiting queues, dispatch/preemption with context save/restore
* **Interrupt Manager** — SYSTEM_CALL, TIMER, IO_COMPLETE, ERROR phases, pending queue, log, dispatch
* **System Call Manager** — READ, WRITE, ALLOCATE, EXIT (stubs for IPC_SEND, IO_REQUEST)
* **Event Log** — 14 event types with listener support
* **Simulation Clock** — cycle counter
* **Program Loader** — text instruction parsing + raw data words
* **Unit Tests** — 9 test files, all passing (CPU, memory, process, interrupt, syscall, types, loader, smoke)

### Implemented (M3: Paging / Virtual Memory)

* **Memory Manager** — page tables per process, frame table, simulated swap, page size 4 / 8 frames (configurable), demand paging (program image stored in swap at creation; pages resident only on first access)
* **Address translation** — logical address → page/offset → frame → physical address; INVALID vs PAGE_FAULT distinction
* **Page fault handling** — PAGE_FAULT interrupt raised by the CPU, dispatched by the Interrupt Manager, page loaded from swap
* **Page replacement** — FIFO baseline with dirty-page write-back to swap (docs/07 section 21)
* **CPU integration** — fetch and LOAD/STORE/READ/WRITE route through the Memory Manager when attached; faulting instructions are retried (fetch faults do not advance PC; execute faults roll back PC to MAR); invalid accesses fail the process via the ERROR interrupt
* **Process Manager integration** — paged createProcess (logical space starts at 0) with legacy flat-Memory fallback preserved; memory released on terminate/fail/reset
* **Memory statistics** — page faults, replacements, used frames/words, swap usage, per-process fault counters
* **Memory events** — PAGE_ALLOCATED, PAGE_ACCESSED, FRAME_ALLOCATED, PAGE_REPLACED, PAGE_LOADED, PAGE_SWAPPED_OUT, PAGE_SWAPPED_IN, INVALID_MEMORY_ACCESS
* **Unit Tests** — test_paging.cpp (14 T-PAGE cases), all passing

### Implemented (M4: Full Interrupt Manager)

* **Interrupt priority queue** — pending interrupts serviced in priority order (docs/05 section 19, finalized during implementation): ERROR > PAGE_FAULT > SYSTEM_CALL > IO_COMPLETE > TIMER; lower value = higher priority. Ordering centralized in `InterruptManager::priorityOf()` so it can be changed in one place
* **Nested-interrupt policy** — interrupts arriving while an ISR executes are queued/deferred, never nested; `serviceNextInterrupt()` refuses to start a second ISR while one is running (docs/05 section 20)
* **FIFO within a priority class** — equal-priority requests are serviced in arrival order (docs/05 section 21)
* **Lifecycle observability** — requests marked SERVICING while dispatched, then COMPLETED (docs/05 section 29); `pendingInterrupts()` accessor exposes the pending queue (docs/05 section 34)
* **Timer** — new Timer component generates periodic TIMER interrupts when a configurable quantum of simulated cycles elapses (default 4, docs/05 section 32 example; 0 disables); basis for Round Robin scheduling (docs/08, Week 5)
* **Unit Tests** — test_timer.cpp (7 T-TIMER cases) + T-INT-009/010/011; T-INT-002 updated from FIFO to priority; full suite 104 test cases / 524 assertions passing

### Implemented (M5: Process Manager Full Features)

* **Process type** — `ProcessType` enum (NORMAL / AI_AGENT) on the PCB; `createAgentProcess()` wrapper (docs/06 section 34); agents are ordinary OS processes
* **CPU-time accounting** — dispatch marks the process on the CPU; the run is charged to `cpuTime` when the process leaves the CPU (preempt, block, terminate, fail, reset); `contextSwitchCount` increments per dispatch
* **Waiting-time accounting** — processes on the READY queue accrue `waitingTime` (charged centrally in the transition layer when leaving READY, including READY -> FAILED)
* **Response and turnaround** — `firstRunCycle` (response time) set on first dispatch; `turnaroundTime = terminated - created`
* **State-change tracking** — `stateChangeCount` incremented on every legal transition
* **Statistics API** — `getProcessStatistics(pid)` (nullopt for unknown pid), `getAllProcessStatistics()`, `cpuUtilization()` (charged CPU time over simulated time, including the current uncharged run); page-fault counts surfaced from the Memory Manager's per-process counters (docs/06 sections 28-29)
* **Unit Tests** — T-PROC-014..023 (type, cpuTime, waitingTime, context switches, response/turnaround, statistics, utilization) + T-PAGE-015 (per-process page faults surfaced in statistics); full suite 115 test cases / 611 assertions passing

---

# 7. Currently In Progress

### Core OS Implementation (M5+)

* Scheduling policies (Week 5)

---

# 8. Not Yet Verified

The following require repository inspection:

* Actual frontend implementation
* REST / WebSocket layer
* AI Agent Layer
* Integration scenarios

---

# 9. Current Blockers

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

1. ✅ Confirm GitHub repository.
2. ✅ Inspect repository structure.
3. ✅ Inspect current source code.
4. ✅ Inspect current branches.
5. ✅ Inspect recent commits.
6. ✅ Compare implementation against documentation.
7. Confirm team assignments.
8. ✅ Commit project governance documents.
9. ✅ Establish implementation baseline.
10. ✅ Complete first Core OS subsystem (CPU/Machine Simulator).

### Next Milestone (Week 3-4)

1. ✅ Implement Paging / Virtual Memory (Memory Manager, Page Tables, Page Faults)
2. ✅ Complete Process Manager (creation data, resource tracking, statistics)
3. ✅ Full Interrupt Manager (timer, page fault, I/O priority queue, nested interrupt policy)
4. ✅ Scheduler interface + FCFS + Round Robin + Priority

---

# 11. Recommended Implementation Order

```text
CPU
  ↓
Process Management
  ↓
Memory / Paging       ← NEXT
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

**2026-08-22 (Mid-review GUI fixes)**

Dashboard reliability pass after first end-to-end testing.

* Build layout consolidated: canonical build directory is backend/build (README Quick Start); stray repository-root build removed
* Port-conflict guard: server probes /health before binding and refuses to start with exit code 1 when another instance already answers - previously a second silent process could squat while the dashboard talked to the stale one (docs/13 section 65)
* Real dashboard statistics: cpuUsage now from ProcessManager::cpuUtilization(), memoryUsage from MemoryManager usedWords/totalWords, pageFaults from Memory Manager counters, scheduler metrics (avg waiting/turnaround/response, throughput) from getAllProcessStatistics() - all were hardcoded zeros before
* Connection honesty in the UI: Engine Connected/Disconnected badge in the Top Bar, disconnection banner with last-known-state notice, Start/Pause buttons only reflect commands the engine actually accepted
* One-screen layout: every panel has a bounded height and internal scrollbar (process table, CPU, frames, page tables, ready queue, interrupts, event log with working category filters); redundant AI-agent card folded into the process table Type column
* Verified live: CPU 98% utilization under Round Robin, page faults incrementing, frame ownership visible, duplicate instance rejected

**2026-08-22 (Mid-Review GUI: REST server + React dashboard)**

Mid-review dashboard complete: the OS engine is now observable and controllable through a web UI.

* C++ REST API server (`aios/server/ApiServer`, cpp-httplib + nlohmann/json) exposing the engine: /health, /api/statistics, /api/processes, /api/agents, /api/cpu, /api/memory, /api/scheduler, /api/interrupts, /api/events, POST /api/command
* Simulation control wired end-to-end: START/PAUSE/STOP gate a 10 cycles/s host loop; RESET restores the initial workload; CHANGE_SCHEDULER maps to SchedulingPolicy
* Standard demo workload per PRD section 20: P1 TextEditor, P2 Compiler, P3 MusicPlayer + A1 Research Agent, A2 Coding Agent (createAgentProcess), 200/150-instruction programs so scheduling is visibly live (~20 s per process)
* Default policy Round Robin with quantum 4 — dashboard shows live context switches; CPU interrupt-manager wiring fixed so demand-paging page faults service and retry instead of failing processes; HALT reaps TERMINATED, CPU-error halts reap FAILED (docs/06)
* Scheduler/MemoryManager public read accessors added for the API layer (currentRunning, readyQueue, pageSize/frameCount); full suite still 131 test cases / 717 assertions passing
* React 19 + TypeScript + Tailwind v4 dashboard (Vite): TopBar controls + live cycle indicator, Sidebar navigation, stat cards, process table with state badges, CPU register panel, physical frame grid, page tables, ready queue, filterable event log, interrupt panel — all values fetched from the REST API (docs/13 sections 7-30, 59)
* Verified end-to-end on port 8081: cycle advancing, RR preemption cycling all five workloads, page fault -> frame load -> instruction retry visible in the event log

**2026-08-20 (M5-Scheduler)**

Scheduler complete: FCFS, Round Robin and Priority policies.

* SchedulingPolicy (FCFS / ROUND_ROBIN / PRIORITY) with policy name accessor; larger priority value = higher priority (preemptive), consistent with docs/08 section 13
* Persistent ready order reconciled with the Process Manager's READY set on every decision (blocked/terminated pids dropped, newly READY pids appended in PID order)
* FCFS: non-preemptive FIFO; Round Robin: quantum expiry preempts the running process to the back of the queue (docs/08 sections 7-10); Priority: preempts only on strictly higher priority, tie-breaks by ready time then PID
* Scheduler events: SCHEDULER_STARTED (policy change), PROCESS_SELECTED, PROCESS_PREEMPTED, TIME_QUANTUM_EXPIRED (docs/08 section 40)
* Decision history (cycle, policy, selected/preempted pid, reason), context-switch count, reset(); TIMER interrupt wired through InterruptManager::setScheduler() for Round Robin
* Unit tests: test_scheduler.cpp (16 T-SCHED cases); full suite 131 test cases / 717 assertions passing
* Build verified with MinGW-w64 GCC 16.2.0; committed and pushed to GitHub

**2026-08-20 (M5)**

Process Manager full features complete (creation data, resource tracking, statistics).

* ProcessType (NORMAL / AI_AGENT) on the PCB + createAgentProcess() wrapper
* CPU-time accounting charged on CPU release (preempt/block/terminate/fail/reset); waiting-time accounting charged centrally when leaving the READY queue
* Response time (first run), turnaround time (terminated - created), state-change and context-switch counters
* Statistics API: getProcessStatistics() / getAllProcessStatistics() / cpuUtilization(); per-process page faults surfaced from the Memory Manager
* Unit tests: T-PROC-014..023 + T-PAGE-015; full suite 115 test cases / 611 assertions passing
* Build verified with MinGW-w64 GCC 16.2.0; committed and pushed to GitHub

**2026-08-20 (M4)**

Full Interrupt Manager complete.

* Interrupt priority queue: pending interrupts serviced in priority order (ERROR > PAGE_FAULT > SYSTEM_CALL > IO_COMPLETE > TIMER), FIFO within equal priority; ordering centralized in InterruptManager::priorityOf()
* Nested-interrupt policy: interrupts arriving while an ISR executes are deferred, never nested
* Timer component: periodic TIMER interrupts on a configurable quantum (default 4 cycles, docs/05 section 32; 0 disables); basis for Round Robin (docs/08)
* Lifecycle observability (SERVICING -> COMPLETED) and pendingInterrupts() accessor (docs/05 section 34)
* Unit tests: test_timer.cpp (7 T-TIMER cases) + T-INT-009/010/011; T-INT-002 updated from FIFO to priority; full suite 104 test cases / 524 assertions passing
* docs/05 section 19 updated with the finalized priority ordering
* Build verified with MinGW-w64 GCC 16.2.0; committed and pushed to GitHub

**2026-08-20 (M3)**

M3 Paging / Virtual Memory complete.

* Memory Manager: page tables, frame table, simulated swap, demand paging, FIFO page replacement with dirty write-back
* Address translation with INVALID vs PAGE_FAULT distinction; MemoryAccessResult API
* PAGE_FAULT interrupt type added; Interrupt Manager dispatches faults to the Memory Manager
* CPU integrated: paged fetch/execute with instruction retry (fetch fault does not advance PC; execute fault rolls back to MAR); invalid access fails the process via ERROR interrupt
* Process Manager integrated: paged createProcess (logical base 0) with legacy flat-Memory fallback; memory released on terminate/fail/reset
* Memory statistics (faults, replacements, used frames/words, swap usage) and 8 new memory event types
* Unit tests: 14 new T-PAGE cases; full suite 94 test cases / 442 assertions passing
* Build verified with MinGW-w64 GCC 16.2.0; committed and pushed to GitHub

**2026-08-19**

M1+M2 Core OS Implementation complete.

* CPU Simulator: registers, fetch-decode-execute, full instruction set, FLAGS, error detection
* Memory: 1024-word flat RAM, program loading, bounds checking
* Process Manager: PCB, state machine, queues, dispatch/preemption with context save/restore
* Interrupt Manager: SYSTEM_CALL, TIMER, IO_COMPLETE, ERROR, pending queue + log
* System Call Manager: READ, WRITE, ALLOCATE, EXIT (stubs for IPC, I/O)
* Event Log, Simulation Clock, Program Loader
* All unit tests passing (9 test files)
* CMake + MinGW build verified
* GitHub repository confirmed and synced

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
868bd48 feat(gui): add React dashboard for the mid review
```

---

# 15. Current Status for README

The README should contain only a concise version of this state:

```text
AIOS is currently in the Core OS Implementation phase.

M1 (Foundation), M2 (Stage I: CPU/System Calls), M3 (Paging/Virtual Memory),
M4 (Full Interrupt Manager), M5 (Process Manager Full Features),
M5-Scheduler (FCFS / Round Robin / Priority) and the Mid-Review GUI
(REST server + React dashboard) are complete:
- CPU Simulator with registers, fetch-decode-execute, full instruction set
- Memory (flat RAM) + Memory Manager (page tables, frames, swap, FIFO replacement)
- Process Manager (PCB, states, queues, context switch, resource tracking, statistics)
- Interrupt Manager (priority queue, timer, nested policy, PAGE_FAULT dispatch) + System Call Manager
- Scheduler (FCFS, Round Robin with quantum preemption, preemptive Priority, decision history)
- REST API server exposing the engine + simulation control (START/PAUSE/STOP/RESET)
- React dashboard: stat cards, process table, CPU panel, frame grid, page tables,
  ready queue, event log, interrupt panel — live data on port 8081/5173
- All unit tests passing (131 test cases), CMake + MinGW build verified

The architecture, OS subsystems, AI-agent layer, GUI direction,
demo scenarios and testing strategy have been defined.

Next:
1. Synchronization (Week 6)
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