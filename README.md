# AIOS — AI-Powered Multiprogramming Operating System Simulator

> A C++-based educational operating-system simulator that demonstrates classical OS mechanisms and extends them with simulated AI-agent workloads.

---

## 🧠 What is AIOS?

**AIOS** is an interactive multiprogramming Operating System simulator designed to make core OS concepts observable and demonstrable.

Instead of implementing each Operating System concept as an isolated program, AIOS combines them into a single simulated environment:

```text
Traditional Processes
        +
Simulated AI Agents
        ↓
   Process Manager
        ↓
     Scheduler
        ↓
        CPU
        ↓
 ┌──────┼──────┐
 ↓      ↓      ↓
Memory  I/O    IPC
 ↓      ↓      ↓
Paging  Buffering
 ↓      ↓
Page    Spooling
Faults
        ↓
Interrupts
        ↓
Synchronization
        ↓
AI Agent Management
        ↓
Checkpoint / Failure / Recovery

The system is entirely simulated. It does **not** attempt to replace the host operating system or control real hardware.
```
---
## 🎯 Problem Statement

Traditional OS simulators demonstrate concepts such as process management, CPU scheduling, memory management, interrupts, IPC and I/O.

AIOS explores an additional question:

> **How can an operating system manage AI-agent workloads using classical OS mechanisms while introducing AI-oriented resource management, adaptive scheduling, communication and recovery?**

The key idea is that AI agents do not bypass the operating system.

They are represented as **OS-managed processes**.

```text
Research Agent
      ↓
     PID
      ↓
     PCB
      ↓
   Scheduler
      ↓
      CPU
```

---

## ✨ Key Features

### Core Operating System

* CPU / Machine simulation
* Machine instructions
* Supervisor calls
* Interrupt handling
* Process creation and termination
* Process states
* PCB-based process management
* Context switching
* Multiprogramming
* FCFS scheduling
* Round Robin scheduling
* Priority scheduling
* Paging
* Page tables
* Virtual memory
* Page faults
* Page replacement
* Synchronization
* Mutex
* Semaphore
* Race-condition demonstration
* Inter-Process Communication
* Message queues
* Blocking and non-blocking IPC
* I/O simulation
* Buffering
* Spooling
* Error handling

### 🤖 AI Extension

AIOS introduces simulated AI-agent workloads.

Initial example agents include:

* Research Agent
* Coding Agent

The AI layer provides:

* AI-agent process management
* Adaptive scheduling
* Explainable scheduling decisions
* Resource quotas
* Permissions
* Agent-to-agent IPC
* Checkpointing
* Simulated agent failure
* Agent recovery

The AI layer extends the OS rather than replacing it.

---

## 🏗️ Architecture

```text
┌─────────────────────────────────────────────┐
│                  USER                       │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│             React Dashboard                 │
│                                             │
│ CPU │ Processes │ Memory │ Scheduler        │
│ IPC │ I/O │ AI Agents │ Logs │ Metrics      │
└──────────────────────┬──────────────────────┘
                       │
                 REST / WebSocket
                       │
                       ▼
┌─────────────────────────────────────────────┐
│              C++ AIOS ENGINE                │
│                                             │
│ CPU / Machine Simulator                    │
│ Process Manager                             │
│ Scheduler                                   │
│ Memory Manager                              │
│ Interrupt Manager                           │
│ System Call Manager                         │
│ Synchronization Manager                     │
│ IPC Manager                                 │
│ I/O Manager                                 │
│ Error Handler                               │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│              AI EXTENSION                   │
│                                             │
│ AI Agent Manager                            │
│ AI Adaptive Scheduler                       │
│ Resource Management                         │
│ Permissions                                 │
│ Agent Communication                         │
│ Checkpoint / Recovery                       │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│          SIMULATED HARDWARE                 │
│                                             │
│ CPU │ RAM │ Pages │ Frames │ Swap │ Devices │
└─────────────────────────────────────────────┘
```

### Source of Truth

The **C++ simulation engine** is the source of truth for the simulated OS state.

The React application is the:

* Visualization layer
* Control layer
* Monitoring interface

The GUI must not contain the core OS logic.

---



The exact implementation structure should be verified against the repository before development is assumed to be complete.

---

## 🛠️ Technology Stack

| Component            | Technology          |
| -------------------- | ------------------- |
| OS Simulation Engine | C++                 |
| Frontend             | React + TypeScript  |
| Styling              | Tailwind CSS        |
| Visualization        | Recharts / Chart.js |
| Backend ↔ Frontend   | REST / WebSocket    |
| Configuration        | JSON                |
| Frontend Build       | Vite                |
| Version Control      | Git + GitHub        |

---

## 🤖 AI Scheduling

AIOS includes an explainable adaptive scheduler.

The scheduler can consider factors such as:

```text
Priority
Waiting Time
Task Urgency
CPU Usage
Memory Usage
Resource Requirements
Page-Fault Activity
```

Conceptually:

```text
             Process / Agent
                    ↓
          ┌────────────────────┐
          │ AI Scheduler       │
          │                    │
          │ Priority           │
          │ Waiting Time       │
          │ Urgency            │
          │ Resource Cost      │
          └─────────┬──────────┘
                    ↓
              Score / Decision
                    ↓
             Selected Process
```

The scheduler should also provide an explanation for its decision rather than behaving as an opaque black box.

---

## 💾 Memory Simulation

AIOS simulates:

```text
Logical Address
      ↓
Page Number + Offset
      ↓
Page Table
      ↓
Frame
      ↓
Physical Address
```

A page fault follows:

```text
Memory Access
      ↓
Page Not Present
      ↓
Page Fault
      ↓
Interrupt
      ↓
Memory Manager
      ↓
Load / Replace Page
      ↓
Update Page Table
      ↓
Resume Process
```

---

## 🔄 Process Lifecycle

Processes follow:

```text
NEW
 ↓
READY
 ↓
RUNNING
 ↓
WAITING
 ↓
READY
 ↓
RUNNING
 ↓
TERMINATED
```

Not every process must visit every state.

For example, an I/O operation may cause:

```text
RUNNING
   ↓
WAITING
   ↓
I/O Completion
   ↓
READY
```

---

## 📡 IPC

Processes and AI agents communicate through the simulated IPC subsystem.

```text
Sender
   ↓
IPC Manager
   ↓
Message Queue
   ↓
Receiver
```

Blocking communication can cause:

```text
Receiver
   ↓
No Message
   ↓
WAITING
   ↓
Message Arrives
   ↓
READY
```

This allows IPC to interact with the process scheduler and process-state system.

---

## 🤖 AI Agent Lifecycle

AI agents follow the normal OS process lifecycle.

Additional AI-specific behaviour can include:

```text
Agent Created
      ↓
OS Process Created
      ↓
READY
      ↓
RUNNING
      ↓
Checkpoint
      ↓
Failure
      ↓
Recovery
      ↓
READY
      ↓
RUNNING
```

Agents must use OS mechanisms for CPU, memory, IPC and scheduling.

---

## 🖥️ Dashboard

The React dashboard is intended to make internal OS operations visible.

Planned views include:

```text
┌─────────────────────────────────────────────┐
│ AIOS Dashboard                              │
├─────────────────────────────────────────────┤
│ CPU                                         │
│ Current Process │ Registers │ Utilization   │
├─────────────────────────────────────────────┤
│ Processes                                   │
│ PID │ Name │ State │ Priority │ CPU         │
├─────────────────────────────────────────────┤
│ Memory                                      │
│ Pages │ Frames │ Page Faults │ Swap         │
├─────────────────────────────────────────────┤
│ Scheduler                                   │
│ Algorithm │ Ready Queue │ Decision │ Score  │
├─────────────────────────────────────────────┤
│ AI Agents                                   │
│ Agent │ State │ Resources │ Checkpoint      │
├─────────────────────────────────────────────┤
│ IPC / I/O / Interrupts / Events             │
├─────────────────────────────────────────────┤
│ Metrics                                     │
└─────────────────────────────────────────────┘
```

---

## 🎬 Demonstration

The primary integrated demonstration is:

```text
Create 3 Processes
        +
Create 2 AI Agents
        ↓
Load into Memory
        ↓
Start Scheduler
        ↓
CPU Execution
        ↓
Context Switching
        ↓
Generate Page Fault
        ↓
Handle Interrupt
        ↓
Perform I/O
        ↓
IPC Between Agents
        ↓
Synchronization
        ↓
Spooling
        ↓
Checkpoint Agent
        ↓
Simulate Agent Failure
        ↓
Recover Agent
        ↓
Resume Execution
        ↓
Compare Scheduling Algorithms
        ↓
Show Results
```

The project also uses smaller predefined scenarios for individual concepts.

---

## 🎯 Development Strategy

AIOS is developed incrementally.

```text
1. CPU / Machine
       ↓
2. Process Management
       ↓
3. Memory / Paging
       ↓
4. Interrupts / System Calls
       ↓
5. Scheduling
       ↓
6. Synchronization
       ↓
7. IPC
       ↓
8. I/O / Buffering / Spooling
       ↓
9. AI Agent Layer
       ↓
10. REST / WebSocket
       ↓
11. React Dashboard
       ↓
12. Integration / Testing
```

The core OS should be stable before significant GUI work is performed.

---

## 📚 Documentation

Start with:

* [`PRD.md`](docs/PRD.md) — Product requirements and scope
* [`AGENTS.md`](AGENTS.md) — Development-agent workflow and project rules
* [`TEAMMATES.md`](TEAMMATES.md) — Team ownership, collaboration and handoffs
* [`STATUS.md`](STATUS.md) — Living project status

Detailed technical specifications are maintained under [`docs/`](docs/).

---

## 🚫 Scope Boundaries

AIOS does **not** initially attempt to implement:

* A real operating-system kernel
* Real hardware control
* Real autonomous AI agents
* A real LLM controlling the CPU scheduler
* Real GPU scheduling
* A distributed operating system
* Production-grade security
* Real hardware drivers

AIOS is an educational simulator.

---

## 💡 Why AIOS?

The project combines classical Operating System concepts with a modern workload model.

The distinction is:

```text
Traditional OS Simulator

Processes
   ↓
Scheduler
   ↓
CPU
```

versus:

```text
AIOS

Processes + AI Agents
        ↓
   Process Manager
        ↓
Classical + Adaptive Scheduler
        ↓
CPU / Memory / IPC / I/O
        ↓
Agent Resource Management
        ↓
Checkpoint / Failure / Recovery
```

The goal is not to put the word "AI" on an ordinary OS simulator.

The goal is to study how **AI-oriented workloads can be treated as OS-managed workloads**.

---

# 📊 Current Status

<!--
IMPORTANT:
This is the only README section that should be continuously updated
by the project development workflow.

Keep this section concise.
Detailed history belongs in STATUS.md.
-->

**Phase:** 🟢 Core OS Implementation — M1/M2/M3 + M4 (Full Interrupt Manager) + M5 (Process Manager Full Features) + M5-Scheduler (FCFS / Round Robin / Priority) Complete

**Current State:**

* Core project concept defined.
* Problem statement defined.
* OS subsystem architecture defined.
* AI-agent layer defined.
* GUI direction defined.
* Technology stack defined.
* Integrated demo flow defined.
* Project scope boundaries defined.
* PRD structure established.
* Agent workflow established.
* Team/status tracking established.
* Canonical repository structure established.
* **GitHub repository confirmed: https://github.com/ManthanGadiya/AIOS.git**
* **M1 (Foundation) + M2 (Stage I: CPU/System Calls) + M3 (Paging/Virtual Memory) + M4 (Full Interrupt Manager) + M5 (Process Manager Full Features) + M5-Scheduler (FCFS / Round Robin / Priority) implemented and tested**

**Implemented (M1+M2):**

* **CPU Simulator** — registers (PC, IR, ACC, MAR, MBR, FLAGS), fetch-decode-execute, full instruction set (LOAD, STORE, ADD, SUB, MUL, DIV, JMP, JZ, READ, WRITE, SYSCALL, HALT), arithmetic flags (ZERO, NEG, CARRY, OVERFLOW, ERROR), error detection
* **Memory** — 1024-word flat RAM, program loading, bounds checking
* **Process Manager** — PCB, state machine (NEW/READY/RUNNING/WAITING/TERMINATED/FAILED), ready/waiting queues, dispatch/preemption with context save/restore
* **Interrupt Manager** — SYSTEM_CALL, TIMER, IO_COMPLETE, ERROR phases, pending queue + log
* **System Call Manager** — READ, WRITE, ALLOCATE, EXIT (stubs for IPC_SEND, IO_REQUEST)
* **Event Log, Simulation Clock, Program Loader**
* **All unit tests passing** (10 test files), **CMake + MinGW-w64 GCC 16.2.0 build verified**

**Implemented (M3: Paging / Virtual Memory):**

* **Memory Manager** — per-process page tables, frame table, simulated swap, demand paging (program image stored in swap at creation; pages loaded on first access), page size 4 / 8 frames (configurable)
* **Address translation** — logical address → page/offset → frame → physical address, with INVALID vs PAGE_FAULT distinction
* **Page faults** — CPU raises PAGE_FAULT interrupt; Interrupt Manager dispatches; page loaded from swap; faulting instruction retried (fetch fault leaves PC unchanged; execute fault rolls back to MAR)
* **Page replacement** — FIFO baseline with dirty-page write-back to swap
* **Invalid access** — out-of-bounds logical access fails the process via the ERROR interrupt
* **Memory statistics and events** — fault/replacement counters, used frames/words, swap usage, per-process fault counters, 8 new event types

**Implemented (M4: Full Interrupt Manager):**

* **Interrupt priority queue** — pending interrupts serviced in priority order (ERROR > PAGE_FAULT > SYSTEM_CALL > IO_COMPLETE > TIMER), FIFO within equal priority; ordering centralized in `InterruptManager::priorityOf()`
* **Nested-interrupt policy** — interrupts arriving while an ISR executes are deferred, never nested
* **Timer** — periodic TIMER interrupts on a configurable quantum (default 4 cycles), basis for Round Robin
* **Lifecycle observability** — requests marked SERVICING then COMPLETED; `pendingInterrupts()` accessor

**Implemented (M5: Process Manager Full Features):**

* **Process type** — `ProcessType` (NORMAL / AI_AGENT) on the PCB; `createAgentProcess()` entry point
* **Resource tracking** — CPU time charged on CPU release (preempt/block/terminate/fail), waiting time charged in the READY queue, response time (first run), turnaround time, state-change and context-switch counters
* **Statistics API** — `getProcessStatistics()` / `getAllProcessStatistics()` / `cpuUtilization()`; per-process page faults surfaced from the Memory Manager

**Implemented (M5-Scheduler):**

* **Scheduling policies** — FCFS (non-preemptive FIFO), Round Robin (quantum expiry preempts and rotates to the back), Priority (larger value = higher priority, preemptive; tie-breaks by ready time then PID)
* **Scheduler state** — persistent ready order reconciled with the Process Manager's READY set on every decision; policy change emits SCHEDULER_STARTED
* **Scheduler events** — PROCESS_SELECTED / PROCESS_PREEMPTED / TIME_QUANTUM_EXPIRED (docs/08 section 40); decision history (cycle, policy, selected/preempted pid, reason), context-switch count, reset()
* **Timer integration** — TIMER interrupt drives Round Robin preemption through `InterruptManager::setScheduler()` (docs/08 section 24)

**Currently Working On:**

1. Synchronization (Week 6)

**Blocked By:**

* Final teammate/responsibility mapping has not yet been confirmed.

### Quick Start

```bash
git clone https://github.com/ManthanGadiya/AIOS.git
cd AIOS/backend
cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER="C:/Users/Admin/scoop/apps/mingw/current/bin/g++.exe" -B build .
cmake --build build --config Debug
# Add the MinGW runtime to PATH so the test binary finds its DLLs:
export PATH="/c/Users/Admin/scoop/apps/mingw/current/bin:$PATH"
./build/tests/aios_tests.exe
```

---

## 🧠 Project Principle

> **The OS manages the workload.
> The AI layer describes and extends the workload.
> The GUI visualizes the workload.**

AIOS should always preserve this separation.

