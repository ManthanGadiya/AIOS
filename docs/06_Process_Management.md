# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 6. Process Management

## 1. Purpose

This document defines how AIOS represents and manages processes throughout their lifecycle.

The Process Management subsystem is responsible for:

* Process creation
* Process identification
* Process states
* Process Control Blocks (PCB)
* Process queues
* CPU allocation
* Context switching
* Process blocking and waking
* Process termination
* Process resource tracking
* Integration with AI agents

The Process Manager acts as the central component connecting the CPU, Scheduler, Memory Manager, Interrupt Manager, IPC Manager, and AI Agent Manager.

---

# 2. Process Concept

A process is a program that is currently being executed or is ready to execute.

In AIOS, a process consists of:

```text
Program
   +
Execution State
   +
CPU Context
   +
Memory
   +
OS Management Information
```

Example:

```text
Program
   │
   ▼
Process
   │
   ├── PID
   ├── State
   ├── CPU Context
   ├── Memory
   ├── Priority
   └── Resources
```

---

# 3. Types of Workloads

AIOS supports two types of workloads:

```text
Workloads
│
├── Normal Processes
│
└── AI Agents
```

Example:

```text
PID 1 → Calculator Process
PID 2 → Sorting Process
PID 3 → File Processing Process

PID 4 → Research Agent
PID 5 → Coding Agent
```

Both are managed by the operating system's process management mechanisms.

---

# 4. Process Lifecycle

Each process follows a defined lifecycle.

```text
              ┌─────────┐
              │   NEW   │
              └────┬────┘
                   │
                   ▼
              ┌─────────┐
              │  READY  │◄─────────────┐
              └────┬────┘              │
                   │                   │
              Scheduler               │
                   │                   │
                   ▼                   │
             ┌──────────┐              │
             │ RUNNING  │              │
             └────┬─────┘              │
                  │                    │
        ┌─────────┼─────────┐          │
        │         │         │          │
        ▼         ▼         ▼          │
     I/O Wait   Exit     Preemption    │
        │         │         │          │
        ▼         ▼         ▼          │
   ┌─────────┐ TERMINATED  READY ──────┘
   │ WAITING │
   └────┬────┘
        │
     I/O Complete
        │
        └──────────────► READY
```

---

# 5. Process States

AIOS uses the following primary states.

## 5.1 NEW

The process has been created but has not yet entered the ready queue.

```text
NEW
 ↓
Process initialization
 ↓
PCB creation
 ↓
Memory allocation
 ↓
READY
```

---

## 5.2 READY

The process is ready to execute but is waiting for CPU allocation.

Example:

```text
Ready Queue

P1
P2
P3
A1
A2
```

Only one process can be in the simulated CPU at a time.

---

## 5.3 RUNNING

The process currently owns the simulated CPU.

```text
CPU
 │
 ▼
P2 RUNNING
```

The CPU executes instructions belonging to the running process.

---

## 5.4 WAITING

The process cannot continue until an event occurs.

Typical reasons:

* I/O request
* IPC wait
* Synchronization
* Resource availability

Example:

```text
P2
 │
 ▼
I/O Request
 │
 ▼
WAITING
```

---

## 5.5 TERMINATED

The process has completed execution or has been terminated because of an unrecoverable error.

```text
RUNNING
   │
   ▼
HALT / Termination
   │
   ▼
TERMINATED
```

---

# 6. Process Control Block

The Process Control Block is the primary data structure used by the OS to manage a process.

Conceptually:

```text
PCB
│
├── Identification
│   ├── PID
│   └── Process Name
│
├── Execution
│   ├── State
│   ├── Program Counter
│   ├── Instruction Register
│   ├── Accumulator
│   ├── MAR
│   ├── MBR
│   └── FLAGS
│
├── Scheduling
│   ├── Priority
│   ├── CPU Time
│   ├── Waiting Time
│   └── Time Quantum
│
├── Memory
│   ├── Page Table Reference
│   ├── Pages
│   └── Memory Requirement
│
└── Resource Information
    ├── I/O State
    └── IPC Information
```

The exact C++ representation will be defined during implementation.

---

# 7. Process Identification

Every process shall have a unique Process ID.

Example:

```text
PID 1
PID 2
PID 3
PID 4
PID 5
```

The Process Manager shall ensure that two active processes do not have the same PID.

For AI agents:

```text
PID = OS process identifier
Agent ID = AI-specific identifier
```

Example:

```text
PID: 4
Agent ID: AGENT-001
Type: Research Agent
```

---

# 8. PCB and CPU Context

The CPU registers represent the currently executing process.

When a process is removed from the CPU, its CPU context must be saved.

```text
CPU
│
├── PC
├── IR
├── ACC
├── MAR
├── MBR
└── FLAGS
```

The context is stored in the process's PCB.

Example:

```text
P1 Running
     │
     ▼
Save CPU Context
     │
     ▼
P1 PCB
```

When P1 runs again:

```text
P1 PCB
     │
     ▼
Load CPU Context
     │
     ▼
CPU
```

---

# 9. Process Creation

A new process is created through the Process Manager.

General flow:

```text
Create Request
      │
      ▼
Process Manager
      │
      ▼
Generate PID
      │
      ▼
Create PCB
      │
      ▼
Load Program
      │
      ▼
Request Memory
      │
      ▼
Initialize CPU Context
      │
      ▼
Process → READY
```

The process is then inserted into the appropriate ready queue.

---

# 10. Process Creation Data

A process creation request may contain:

```text
Process Creation Request
├── Process Name
├── Program
├── Priority
├── Memory Requirement
└── Resource Requirements
```

For an AI agent:

```text
Agent Creation Request
├── Agent Name
├── Agent Type
├── Workload
├── Priority
├── Memory Requirement
├── Resource Requirements
├── Token Budget
└── Tool Permissions
```

---

# 11. Process Termination

A process may terminate because:

* It executes `HALT`.
* It executes an `EXIT` system call.
* It encounters an unrecoverable error.
* The user terminates it.
* The OS terminates it because of a configured resource policy.

General flow:

```text
Process Running
      │
      ▼
Termination Event
      │
      ▼
Save / Record Final State
      │
      ▼
Release Resources
      │
      ▼
Release Memory
      │
      ▼
Remove from Active Process Table
      │
      ▼
TERMINATED
```

---

# 12. Process Queues

AIOS shall maintain logical queues for processes.

Primary queues:

```text
Process Queues
│
├── Ready Queue
├── Waiting Queue
└── Terminated / History
```

The Ready Queue contains processes eligible for CPU allocation.

The Waiting Queue contains processes waiting for an event.

---

# 13. Ready Queue

The Ready Queue contains processes in the READY state.

Example:

```text
Ready Queue

┌────┬────┬────┬────┬────┐
│ P1 │ P2 │ P3 │ A1 │ A2 │
└────┴────┴────┴────┴────┘
```

The Scheduler selects a process from this queue.

The structure used internally depends on the scheduling algorithm.

Examples:

```text
FCFS        → FIFO Queue
Round Robin → Circular Queue
Priority    → Priority Queue
```

---

# 14. Waiting Queue

Processes waiting for an event are removed from the ready queue.

Example:

```text
P2
 │
 ▼
I/O Request
 │
 ▼
WAITING
 │
 ▼
Waiting Queue
```

When the event completes:

```text
I/O Complete
     │
     ▼
P2 → READY
     │
     ▼
Ready Queue
```

---

# 15. Process State Transitions

The Process Manager shall control valid state transitions.

### Creation

```text
NEW → READY
```

### Scheduling

```text
READY → RUNNING
```

### Preemption

```text
RUNNING → READY
```

### I/O / Resource Wait

```text
RUNNING → WAITING
```

### Event Completion

```text
WAITING → READY
```

### Successful Completion

```text
RUNNING → TERMINATED
```

### Unrecoverable Error

```text
RUNNING → TERMINATED
```

Invalid transitions should be rejected by the Process Manager.

---

# 16. Context Switching

Context switching occurs when the CPU changes from one process to another.

Example:

```text
P1 Running
    │
    ▼
Save P1 Context
    │
    ▼
Update P1 PCB
    │
    ▼
Scheduler
    │
    ▼
Select P2
    │
    ▼
Load P2 Context
    │
    ▼
Update P2 State
    │
    ▼
P2 Running
```

---

# 17. Causes of Context Switching

A context switch may occur because of:

```text
Timer Interrupt
Time Quantum Expired
Process Blocks
Process Terminates
Higher-Priority Process
Manual Scheduling Event
```

The exact behaviour depends on the selected scheduling policy.

---

# 18. Process Resource Tracking

The Process Manager shall maintain information about resources used by processes.

Possible tracked values:

```text
CPU Time
Memory
I/O Requests
IPC Operations
```

For AI agents, additional values may include:

```text
Token Budget
Tool Usage
Agent-Specific Resource Usage
```

---

# 19. AI Agent as a Process

An AI agent is represented as a process with additional metadata.

Conceptually:

```text
                 AI Agent
                    │
                    ▼
              AI Agent Manager
                    │
                    ▼
             Process Manager
                    │
                    ▼
                   PCB
                    +
              AI Metadata
```

The AI agent therefore participates in:

* Scheduling
* Memory management
* Context switching
* IPC
* Synchronization
* I/O
* Error handling

---

# 20. AI Agent Metadata

An AI agent may contain:

```text
AI Agent Metadata
│
├── Agent ID
├── Agent Type
├── Token Budget
├── Tool Permissions
├── Resource Quotas
├── Checkpoint Reference
└── Knowledge Bus Information
```

This information complements rather than replaces the PCB.

---

# 21. Normal Process vs AI Agent

| Property               | Normal Process | AI Agent |
| ---------------------- | -------------- | -------- |
| PID                    | Yes            | Yes      |
| PCB                    | Yes            | Yes      |
| CPU                    | Yes            | Yes      |
| Memory                 | Yes            | Yes      |
| Scheduling             | Yes            | Yes      |
| IPC                    | Yes            | Yes      |
| Synchronization        | Yes            | Yes      |
| I/O                    | Yes            | Yes      |
| Agent ID               | No             | Yes      |
| Token Budget           | No             | Yes      |
| Tool Permissions       | No             | Yes      |
| Checkpoint             | Optional       | Yes      |
| AI Scheduling Metadata | No             | Yes      |

The underlying OS mechanisms remain shared.

---

# 22. Process and Memory Relationship

Every process requires memory.

The Process Manager requests memory from the Memory Manager.

```text
Process Manager
      │
      │ Memory Request
      ▼
Memory Manager
      │
      ▼
Page Allocation
      │
      ▼
Page Table
      │
      ▼
Process PCB
```

The PCB stores a reference to the process's memory information.

The Process Manager does not directly manipulate physical frames.

---

# 23. Process and Scheduler Relationship

The Scheduler selects processes from the ready queue.

```text
Process Manager
      │
      ▼
Ready Queue
      │
      ▼
Scheduler
      │
      ▼
Selected Process
      │
      ▼
CPU
```

The Process Manager updates the process state based on the Scheduler's decision.

---

# 24. Process and Interrupt Relationship

Interrupts may change a process's state.

Example:

```text
P1 Running
    │
    ▼
Timer Interrupt
    │
    ▼
Scheduler
    │
    ▼
P1 READY
```

Another example:

```text
P2 Running
    │
    ▼
I/O Request
    │
    ▼
P2 WAITING
```

The Interrupt Manager and Process Manager therefore work closely together.

---

# 25. Process and IPC Relationship

A process may send or receive messages through IPC.

Example:

```text
P1
 │
 │ Send Message
 ▼
IPC Manager
 │
 ▼
P2
```

If P2 is waiting for a message:

```text
P2 WAITING
     │
     ▼
Message Arrives
     │
     ▼
P2 READY
```

---

# 26. Process and Synchronization

A process may need to acquire a synchronization mechanism before accessing a shared resource.

Example:

```text
P1
 │
 ▼
Request Mutex
 │
 ├── Available ──► Critical Section
 │
 └── Busy ───────► WAITING
```

When the mutex becomes available:

```text
Mutex Released
     │
     ▼
Waiting Process
     │
     ▼
READY
```

---

# 27. Process Control Table

The Process Manager shall maintain an active process table.

Example:

```text
┌─────┬──────────────┬──────────┬──────────┬──────────┐
│ PID │ Name         │ State    │ Priority │ Type     │
├─────┼──────────────┼──────────┼──────────┼──────────┤
│ 1   │ Process_A    │ READY    │ 3        │ Normal   │
│ 2   │ Process_B    │ RUNNING  │ 5        │ Normal   │
│ 3   │ Process_C    │ WAITING  │ 2        │ Normal   │
│ 4   │ Research     │ READY    │ 7        │ AI Agent │
│ 5   │ Coding       │ READY    │ 6        │ AI Agent │
└─────┴──────────────┴──────────┴──────────┴──────────┘
```

The frontend can visualize this table.

---

# 28. Process Statistics

AIOS shall maintain process execution statistics.

Initial statistics:

```text
PID
Arrival Time
CPU Time
Waiting Time
Execution Time
State Changes
Context Switch Count
Page Fault Count
I/O Requests
IPC Operations
```

These statistics will later be used to compare scheduling algorithms.

---

# 29. Process Scheduling Metrics

The Process Manager shall maintain information required to calculate:

### Waiting Time

Time spent waiting in the ready queue.

### Turnaround Time

Time from process creation/arrival until completion.

### Response Time

Time from process arrival until its first CPU execution.

### CPU Utilization

Percentage of simulation time during which the CPU is executing a process.

These metrics are important for comparing FCFS, Round Robin, Priority, and AI Adaptive Scheduling.

---

# 30. Process Priority

Each process shall have a priority value.

Example:

```text
P1 → Priority 3
P2 → Priority 7
P3 → Priority 2
A1 → Priority 8
A2 → Priority 5
```

The interpretation of priority depends on the selected scheduling algorithm.

The AI Scheduler may dynamically calculate an effective scheduling score without permanently changing the base priority.

---

# 31. Process Starvation

A process may remain in the ready queue for an excessive amount of time if other processes repeatedly receive CPU access.

AIOS shall track waiting time so that starvation can be detected during scheduling experiments.

The AI Adaptive Scheduler may use waiting time as one factor in its scheduling heuristic.

Example:

```text
Long Waiting Time
       │
       ▼
Increase Scheduling Preference
       │
       ▼
Process Gets CPU
```

The exact anti-starvation policy will be finalized in the Scheduling document.

---

# 32. Process Fairness

AIOS should avoid allowing a single process or AI agent to monopolize the simulated CPU.

Fairness can be influenced through:

* Round Robin time quantum
* Priority policies
* Waiting-time consideration
* AI scheduling heuristics

The simulator should expose sufficient statistics to evaluate fairness.

---

# 33. Process Termination and Resource Cleanup

When a process terminates, resources associated with it must be released.

```text
Process Termination
       │
       ├── Release CPU
       │
       ├── Release Memory
       │
       ├── Remove IPC State
       │
       ├── Release I/O Requests
       │
       └── Release Synchronization Resources
```

For AI agents, additional cleanup may include:

```text
Release Agent Resources
Update Checkpoint State
Clear Pending Tool Requests
Update Agent Manager
```

---

# 34. Process Manager Interfaces

The Process Manager should expose functionality conceptually similar to:

```text
ProcessManager
│
├── createProcess()
├── createAgentProcess()
├── terminateProcess()
├── blockProcess()
├── wakeProcess()
├── getProcess()
├── getAllProcesses()
├── updateProcessState()
├── saveContext()
├── restoreContext()
├── getReadyQueue()
└── getProcessStatistics()
```

The exact C++ method signatures will be finalized during implementation.

---

# 35. Process Events

The Process Manager shall generate events for important state changes.

Examples:

```text
PROCESS_CREATED
PROCESS_READY
PROCESS_RUNNING
PROCESS_BLOCKED
PROCESS_WAKEUP
CONTEXT_SWITCH
PROCESS_TERMINATED
PROCESS_ERROR
```

Example:

```text
{
    "type": "CONTEXT_SWITCH",
    "from_pid": 1,
    "to_pid": 4,
    "reason": "TIME_QUANTUM_EXPIRED"
}
```

These events can be displayed in the dashboard.

---

# 36. Example: Complete Process Lifecycle

Consider a normal process P1.

```text
Create P1
   │
   ▼
NEW
   │
   ▼
PCB Created
   │
   ▼
Memory Allocated
   │
   ▼
READY
   │
   ▼
Scheduler Selects P1
   │
   ▼
RUNNING
   │
   ▼
CPU Executes
   │
   ├───────────────┐
   │               │
   ▼               ▼
I/O Request      Quantum End
   │               │
   ▼               ▼
WAITING           READY
   │               │
   ▼               │
I/O Complete       │
   │               │
   └──────► READY ◄┘
             │
             ▼
         RUNNING
             │
             ▼
            HALT
             │
             ▼
         TERMINATED
```

---

# 37. Example: AI Agent Lifecycle

Consider the Research Agent.

```text
Create Research Agent
          │
          ▼
      AI Metadata
          │
          ▼
      PCB Created
          │
          ▼
        READY
          │
          ▼
   AI Scheduler Selects
          │
          ▼
       RUNNING
          │
          ├───────────────┐
          │               │
          ▼               ▼
      I/O Request       IPC
          │               │
          ▼               ▼
       WAITING         Continue
          │
          ▼
     I/O Complete
          │
          ▼
        READY
          │
          ▼
       RUNNING
          │
          ▼
    Checkpoint Created
          │
          ▼
    Simulated Failure
          │
          ▼
       Recovery
          │
          ▼
        READY
          │
          ▼
       RUNNING
          │
          ▼
      TERMINATED
```

---

# 38. Process Management and the Final Demo

The Process Manager must support the project's main demonstration:

```text
1. Create 3 normal processes
2. Create 2 AI agents
3. Create PCBs
4. Allocate memory
5. Place workloads in READY state
6. Start Scheduler
7. Run processes on CPU
8. Demonstrate context switching
9. Trigger page fault
10. Block and resume processes
11. Demonstrate IPC
12. Crash an AI agent
13. Recover it from checkpoint
14. Compare scheduling algorithms
```

---

# 39. Success Criteria

The Process Management module is complete when:

* Processes can be created.
* Every process receives a unique PID.
* PCBs are created and maintained.
* Process states are correctly managed.
* Ready and waiting queues work.
* Processes can enter and leave the CPU.
* Context can be saved and restored.
* Context switching works.
* Processes can block and wake.
* Processes can terminate.
* Process resources can be tracked.
* AI agents can be represented as OS-managed processes.
* Process statistics can be collected.
* Process events are logged.
* The frontend can display process information.

---

# 40. Summary

Process Management is the central coordination layer of AIOS.

Its role can be summarized as:

```text
                 Process Manager
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
      PCB           Queues         Lifecycle
        │              │              │
        └──────────────┼──────────────┘
                       │
        ┌──────────────┼───────────────┐
        │              │               │
        ▼              ▼               ▼
    Scheduler       Memory          Interrupts
        │           Manager           │
        ▼              │               ▼
       CPU             │             Events
                       │
                       ▼
                    Process
```

The Process Manager provides the foundation on which scheduling, multiprogramming, memory management, synchronization, IPC, I/O, and AI agent management operate.

The next subsystem is **Paging and Virtual Memory**, which will define exactly how each process gets logical memory, how pages and frames are represented, how page tables work, how address translation occurs, how page faults are generated, and how pages are moved between RAM and simulated swap.
