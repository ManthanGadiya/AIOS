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
| Manthan Gadiya | Project Lead + Core OS Lead | CPU, Interrupts, System Calls, Simulation Loop, Integration | Active |
| Pragati Gupta | Memory & Process Manager | Memory / Paging, Process Management, PCB, Statistics | Active |
| Anvay Hande | Scheduling & Synchronization Engineer | Scheduler, Synchronization, AI Adaptive Scheduler | Active |
| Hari Jadhav | Communication & I/O Engineer | IPC, I/O / Buffering / Spooling | Active |
| Pragati Jadhav | AI Agent & Frontend Lead | AI Agent Layer, Checkpoint / Recovery, React Dashboard | Active |

> Team confirmed 2026-09-26. BLOCKER-002 (team assignments) resolved.

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

# 4. Current Allocation — Full 5-Member Distribution

> Retroactive credit: M1-M5 + M5-Scheduler + Mid-Review GUI (till 22 Aug 2026, 131 tests / 717 assertions) is credited below as past work so every member has done something. Future work balances Week 6-8 load ~20% each.

| Member | Role | Work Done Till Now (till 22 Aug) | Files Overseeing | Future Work In Hand (Week 6-8) |
|---|---|---|---|---|
| Manthan Gadiya | Project Lead + Core OS Lead | M1/M2: CPU fetch-decode-execute (12 opcodes LOAD/STORE/ADD/SUB/MUL/DIV/JMP/JZ/READ/WRITE/SYSCALL/HALT), FLAGS, 1024-word flat RAM, SimulationClock, EventLog, ProgramLoader. M4: Interrupt priority ERROR > PAGE_FAULT > SYSTEM_CALL > IO_COMPLETE > TIMER, Timer quantum 4, nested-interrupt deferral, SystemCallManager (READ/WRITE/ALLOCATE/EXIT). | `backend/include/aios/cpu/CPU.hpp`, `backend/src/aios/cpu/CPU.cpp`, `backend/include/aios/interrupt/`, `backend/src/aios/interrupt/`, `backend/include/aios/core/`, `backend/src/aios/server/main.cpp` (simulation loop), `backend/include/aios/demo/Runner.hpp` | Lead Integration & Release: WebSocket live stream, OSState central wiring, Final Demo scenario docs/14 (3P+2A → PageFault → IPC → I/O → Checkpoint → Recover), full regression per docs/15_Testing.md, STATUS/TEAMMATES/README governance, Final Acceptance. |
| Pragati Gupta | Memory & Process Manager | M3: MemoryManager (page tables, frame table, swap, demand paging, FIFO + dirty write-back, INVALID vs PAGE_FAULT). M5: PCB, ProcessType NORMAL/AI_AGENT, 6-state machine NEW/READY/RUNNING/WAITING/TERMINATED/FAILED, resource tracking (cpuTime, waitingTime, response, turnaround, contextSwitch), getProcessStatistics(). | `backend/include/aios/memory/`, `backend/src/aios/memory/`, `backend/include/aios/process/`, `backend/src/aios/process/`, `backend/tests/test_paging.cpp`, `test_memory.cpp`, `test_process.cpp`, frontend MemoryGrid + ProcessTable (memory/process display) | Week 7: Checkpoint memory restore (docs/12 §35), memory quota enforcement (§25), page-replacement tuning (FIFO → optional Clock), memory stats for dashboard. Docs 04, 06, 07 owner. |
| Anvay Hande | Scheduling & Synchronization Engineer | M5-Scheduler: FCFS / RoundRobin / Priority, ready-order reconciliation, TIME_QUANTUM_EXPIRED, decision history + Gantt data, Timer → Scheduler hook via InterruptManager::setScheduler(). | `backend/include/aios/scheduling/Scheduler.hpp`, `backend/src/aios/scheduling/Scheduler.cpp`, `backend/include/aios/interrupt/Timer.hpp`, `backend/tests/test_scheduler.cpp`, `test_timer.cpp`, frontend SchedulerPanel + ReadyQueue (new Sync panel to be added) | Week 6: SynchronizationManager per docs/09 — Mutex/Semaphore, race-condition demo, deadlock detection, wait graph. Week 7: AI Adaptive Scheduler per docs/12 §15 (Score = Priority + Wait + Urgency − Cost) + explainability panel. Docs 08, 09, 12 §15 owner. |
| Hari Jadhav | Communication & I/O Engineer | M1/M2 Infra: EventLog types 23-26 (scheduler events), OSState wiring, helped wire ApiServer core endpoints. Syscall stubs IPC_SEND / IO_REQUEST defined in types.hpp. | `backend/include/aios/core/types.hpp` (IPC/IO event + syscall IDs), future `backend/**/IPCManager.*`, `IOManager.*`, `Buffer.*`, `Spooler.*`, `backend/tests/test_ipc.cpp`, `test_io.cpp` (to be created), frontend IPC + I/O panels (to be created) | Week 6: IPC Manager per docs/10 — queues, SEND/RECEIVE blocking/non-blocking, permissions, capacity. I/O Manager per docs/11 — devices Disk/Printer/Keyboard/Network, buffers, spool queue, IO_COMPLETE interrupt. Docs 10, 11 + IPC/I/O panels owner. |
| Pragati Jadhav | AI Agent & Frontend Lead | Mid-Review GUI: ApiServer /api/* + /health + POST /command, START/PAUSE/STOP/RESET/CHANGE_SCHEDULER, 10 cycles/s loop, port-guard 8081, React dashboard (CPU register strip, 16×4 frame grid, page-table toggle, process table, ready queue, event-log filters, interrupt panel, Scheduler Gantt). | `backend/src/aios/server/ApiServer.*`, `backend/include/aios/server/`, future `backend/**/AIAgentManager.*`, `CheckpointManager.*`, all of `frontend/src/*` (App, pages/Dashboard, components/, services/, types/), `backend/tests/test_agents.cpp` (to be created) | Week 7: AIAgentManager per docs/12 — Research/Coding agents, quotas (CPU/Mem/Token), tool permissions READ/SEARCH/WRITE_CODE, checkpoint CP-001, FAILED → RECOVERED. Week 7/8: Final GUI — AI agent cards, IPC flow A1→A2, I/O viz, charts, demo controls. Docs 12, 13, 14 owner. |

## 4A. Role Explanations — What Each Role Means

### 1. Manthan Gadiya — Project Lead + Core OS Lead
Owns the heart of the simulator: the CPU executes instructions, the Interrupt Manager is the reception bell, and the simulation loop in `main.cpp` ticks everything forward. Past work proves the engine runs (fetch-decode-execute + priority interrupts). Future work is integration: wiring every new manager (Sync/IPC/IO/AI) into the same loop without breaking the 131 passing tests, plus the final integrated demo. If anything crashes the demo, this role fixes it.

### 2. Pragati Gupta — Memory & Process Manager
Owns where processes live and who they are: pages/frames/swap (hotel rooms) and PCB identity (ID card). Past work proves demand paging + page-fault retry and full resource statistics work. Future work is quota enforcement and checkpoint-memory restore — i.e. making sure an agent that recovers gets its pages back. Explains viva questions on address translation and PCB lifecycle.

### 3. Anvay Hande — Scheduling & Synchronization Engineer
Owns who runs next and who waits: FCFS/RR/Priority today, Mutex/Semaphore tomorrow, AI Adaptive scoring next. Past work proves preemption + Gantt history work. Future work is the most demo-visible: race-condition demo (11 vs 12), deadlock detection, and the explainable AI scheduler (Score + Reasons). Explains viva questions on scheduling comparison and mutual exclusion.

### 4. Hari Jadhav — Communication & I/O Engineer
Owns how processes talk and how they use devices: message queues (hotel post) and Disk/Printer/Keyboard/Network + buffers + spool queue (laundry). Past infra work (events, stubs) unblocks this. Future work completes all of Week 6 Stage-III: blocking RECEIVE (WAITING → READY), queue capacity policy, device queues, IO_COMPLETE interrupts. Explains viva questions on blocking IPC and spooling vs buffering.

### 5. Pragati Jadhav — AI Agent & Frontend Lead
Owns the novelty and its proof: AI agents as OS processes + the dashboard that makes the OS visible. Past work proves the full REST + React loop is live on 8081/5173 with real (non-hardcoded) stats. Future work is the AI layer (Research/Coding agents, permissions, checkpoint/crash/recover) plus the final GUI panels (AI cards, IPC flow, I/O viz, charts) and demo controls. Explains viva questions on AI-as-workload and "GUI value == backend value" (PRD Principle 3).

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

## 2026-09-26

* Team confirmed: Manthan Gadiya (Lead + Core OS), Pragati Gupta (Memory & Process), Anvay Hande (Scheduling & Sync), Hari Jadhav (IPC & I/O), Pragati Jadhav (AI & Frontend). BLOCKER-002 resolved.
* §4 replaced with full 5-member distribution table (Member | Role | Work Done Till Now | Files Overseeing | Future Work) + §4A role explanations.

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

1. What are the confirmed names of all five team members? → **Answered 2026-09-26: Manthan Gadiya, Pragati Gupta, Anvay Hande, Hari Jadhav, Pragati Jadhav**
2. What subsystem is assigned to each member? → **Answered 2026-09-26: see §4 table (Manthan → CPU/Interrupts/Integration; Pragati Gupta → Memory/Process; Anvay → Scheduler/Sync; Hari → IPC/IO; Pragati Jadhav → AI/Frontend)**
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