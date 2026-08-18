# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 3. System Architecture

## 1. Purpose

This document defines the high-level architecture of AIOS and describes how its major components interact.

AIOS follows a layered architecture in which the simulated hardware forms the lowest layer, the OS kernel provides process and resource management, the AI layer extends the kernel for AI-oriented workloads, and the frontend provides visualization and user control.

The architecture is designed around one primary principle:

> **The C++ OS Engine performs the actual simulation. The React frontend only controls and visualizes the simulation.**

---

# 2. Architectural Overview

AIOS consists of four major layers:

```text
┌──────────────────────────────────────────────────────┐
│                  Presentation Layer                  │
│             React + TypeScript + Tailwind            │
│                                                      │
│ Dashboard │ Process View │ Memory │ CPU │ Events     │
└──────────────────────────┬───────────────────────────┘
                           │
                     REST / WebSocket
                           │
┌──────────────────────────▼───────────────────────────┐
│                 Simulation / OS Layer                │
│                         C++                         │
│                                                      │
│ Process Manager │ Scheduler │ Memory Manager         │
│ Interrupts      │ IPC       │ Synchronization       │
│ System Calls    │ I/O       │ Error Manager         │
└──────────────────────────┬───────────────────────────┘
                           │
┌──────────────────────────▼───────────────────────────┐
│                    AI Extension Layer                │
│                                                      │
│ AI Agent Manager │ AI Scheduler │ Permissions        │
│ Resource Manager │ Knowledge Bus │ Checkpoint Manager │
└──────────────────────────┬───────────────────────────┘
                           │
┌──────────────────────────▼───────────────────────────┐
│                 Simulated Hardware Layer             │
│                                                      │
│ CPU │ Registers │ RAM │ Pages │ Frames │ Swap │ I/O  │
└──────────────────────────────────────────────────────┘
```

The exact internal C++ class structure may evolve during implementation, but the responsibilities and boundaries defined here should remain stable.

---

# 3. Layered Architecture

## 3.1 Layer 1 — Simulated Hardware

This layer represents the hardware that would normally be controlled by an operating system.

It contains:

```text
Simulated Hardware
│
├── CPU
│   ├── PC
│   ├── IR
│   ├── ACC
│   ├── MAR
│   ├── MBR
│   └── FLAGS
│
├── Main Memory
│   ├── Pages
│   └── Frames
│
├── Swap Storage
│
└── Simulated I/O Devices
```

The hardware layer does not decide which process should run.

It simply provides simulated resources that the OS controls.

---

# 4. CPU Simulator

The CPU Simulator is responsible for executing instructions belonging to the currently running process.

The CPU follows the basic:

```text
Fetch → Decode → Execute
```

cycle.

```text
             ┌──────────────┐
             │ Program Counter │
             └───────┬──────┘
                     │
                     ▼
              Fetch Instruction
                     │
                     ▼
              Instruction Register
                     │
                     ▼
                   Decode
                     │
                     ▼
                  Execute
                     │
                     ▼
              Update Registers
                     │
                     ▼
             Next Instruction
```

The CPU does not select the next process.

That responsibility belongs to the Scheduler.

---

# 5. OS Kernel / Simulation Layer

The OS layer is the central control layer of AIOS.

It manages:

```text
OS Layer
│
├── Process Manager
├── Scheduler
├── Memory Manager
├── System Call Manager
├── Interrupt Manager
├── Synchronization Manager
├── IPC Manager
├── I/O Manager
└── Error Manager
```

These components cooperate to simulate the behaviour of a multiprogramming operating system.

---

# 6. Process Manager

The Process Manager controls the lifecycle of processes.

Responsibilities include:

* Process creation
* Process termination
* Process state management
* PCB creation
* PCB updates
* Process resource tracking
* Ready/waiting process management

The Process Manager interacts with:

```text
Process Manager
│
├── Scheduler
├── Memory Manager
├── Interrupt Manager
├── IPC Manager
└── AI Agent Manager
```

---

# 7. Process Control Block

Each process is represented using a PCB.

Conceptually:

```text
PCB
│
├── PID
├── Process Name
├── State
├── Program Counter
├── Registers
├── Priority
├── CPU Time
├── Memory Information
├── Page Table Reference
└── Scheduling Information
```

For AI agents, additional information is maintained through the AI Agent Management layer.

---

# 8. Scheduler

The Scheduler determines which ready process receives the CPU.

The scheduler receives information from the Process Manager and selects the next process according to the currently selected scheduling policy.

Supported policies:

```text
Scheduler
│
├── FCFS
├── Round Robin
├── Priority
└── AI Adaptive
```

The Scheduler interacts with the CPU through the Process Manager and performs context switching when required.

---

# 9. Context Switching

A context switch occurs when the CPU stops executing one process and begins executing another.

Example:

```text
P1 Running
     │
     │ Timer / Quantum / Scheduling Event
     ▼
Save P1 Context
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
P2 Running
```

The saved context includes information such as:

* Program Counter
* Registers
* Process state
* Relevant execution information

---

# 10. Memory Manager

The Memory Manager controls simulated memory.

Responsibilities:

* Page allocation
* Frame allocation
* Page table management
* Logical-to-physical address translation
* Page fault detection
* Page replacement
* Swap management

Architecture:

```text
Process
   │
   ▼
Logical Address
   │
   ▼
Page Number + Offset
   │
   ▼
Page Table
   │
   ├── Page Present ──► Frame
   │
   └── Page Missing
             │
             ▼
         Page Fault
             │
             ▼
        Interrupt Manager
```

---

# 11. Interrupt Manager

The Interrupt Manager handles events that require OS intervention.

Interrupt sources include:

```text
Interrupt Manager
│
├── Timer Interrupt
├── System Call
├── Page Fault
├── I/O Completion
└── Error Event
```

General flow:

```text
Event
 │
 ▼
Interrupt Generated
 │
 ▼
Interrupt Manager
 │
 ▼
Identify Interrupt
 │
 ▼
Execute Appropriate Handler
 │
 ▼
Resume / Block / Terminate / Reschedule
```

---

# 12. System Call Manager

Processes cannot directly perform privileged OS operations.

Instead, they request services through system calls.

```text
Process
   │
   │ System Call
   ▼
System Call Manager
   │
   ▼
Kernel Service
   │
   ├── Memory
   ├── I/O
   ├── Process
   └── IPC
   │
   ▼
Return Result
```

The system call mechanism is connected to the interrupt mechanism defined in Stage I.

---

# 13. Synchronization Manager

The Synchronization Manager controls access to shared resources.

Initial mechanisms:

```text
Mutex
Semaphore
```

Example:

```text
Process A ─────┐
               │
               ▼
          Shared Resource
               ▲
               │
Process B ─────┘
```

The synchronization mechanism ensures that conflicting operations are controlled.

---

# 14. IPC Manager

The IPC Manager provides communication between processes.

Conceptually:

```text
IPC Manager
│
├── Message Queue
├── Shared Memory
└── Pipe
```

Example:

```text
Process A
   │
   │ Message
   ▼
IPC Manager
   │
   ▼
Process B
```

AI agents use the same underlying IPC infrastructure.

---

# 15. I/O Manager

The I/O Manager simulates communication with devices.

Example devices:

```text
I/O Manager
│
├── Disk
├── Keyboard
├── Printer
└── Network
```

I/O request flow:

```text
Process
   │
   ▼
I/O Request
   │
   ▼
I/O Manager
   │
   ▼
I/O Queue
   │
   ▼
Simulated Device
   │
   ▼
I/O Completion Event
   │
   ▼
Interrupt Manager
```

---

# 16. Buffer Manager

The Buffer Manager provides temporary storage between processes and simulated devices.

```text
Process
   │
   ▼
Buffer
   │
   ▼
I/O Device
```

The buffer allows the process and device to operate at different simulated speeds.

---

# 17. Spooler

The Spooler manages queued I/O requests.

Example:

```text
P1 ──┐
P2 ──┼──► Spool Queue ──► Printer
P3 ──┘
```

The Spooler stores requests until the simulated device is ready.

---

# 18. Error Manager

The Error Manager handles invalid or exceptional conditions.

Examples:

```text
Invalid Instruction
Invalid Memory Address
Divide by Zero
Invalid System Call
Page Fault
Invalid Process Operation
```

The Error Manager works with the Interrupt Manager and Process Manager.

---

# 19. AI Extension Layer

The AI Extension Layer sits above the core OS simulation.

Its purpose is to extend traditional process management for simulated AI agents.

```text
AI Extension Layer
│
├── AI Agent Manager
├── AI Adaptive Scheduler
├── Resource Manager
├── Permission Manager
├── Knowledge Bus
└── Checkpoint Manager
```

The AI layer uses existing OS mechanisms rather than bypassing them.

---

# 20. AI Agent Manager

The AI Agent Manager creates and manages simulated AI agents.

Example:

```text
AI Agent Manager
│
├── Research Agent
└── Coding Agent
```

Each AI agent is associated with a process and therefore participates in normal OS management.

```text
AI Agent
   │
   ▼
Process Representation
   │
   ▼
PCB + AI Metadata
```

---

# 21. AI Adaptive Scheduler

The AI Scheduler is implemented as an additional scheduling policy.

It receives process and agent information from the OS.

```text
Process Information
       +
Agent Information
       │
       ▼
AI Scheduler
       │
       ├── Priority
       ├── Waiting Time
       ├── CPU Usage
       ├── Memory Usage
       └── Task Urgency
       │
       ▼
Scheduling Score
       │
       ▼
Selected Process
```

The scheduler uses deterministic heuristics rather than an external AI model.

This keeps scheduling explainable during the viva.

---

# 22. Resource Manager

The Resource Manager tracks simulated resources assigned to AI agents.

Resources may include:

```text
CPU Time
Memory
I/O Requests
Token Budget
```

Example:

```text
Research Agent
│
├── CPU: 40 units
├── Memory: 20 units
├── I/O: 5 requests
└── Tokens: 1000
```

The Resource Manager works with the Scheduler and AI Agent Manager.

---

# 23. Permission Manager

The Permission Manager controls access to simulated tools.

Example:

```text
Research Agent
│
├── File System  ✓
├── Internet     ✓
├── Python       ✓
└── Camera       ✗
```

Before an agent performs a tool request:

```text
Agent
 │
 ▼
Permission Manager
 │
 ├── Allowed ──► Tool Request
 │
 └── Denied ───► Error / Rejection
```

---

# 24. Knowledge Bus

The Knowledge Bus provides an AI-oriented communication mechanism.

It is built on top of the OS communication infrastructure.

```text
Agent A
   │
   │ Knowledge
   ▼
Knowledge Bus
   │
   ▼
Agent B
```

The Knowledge Bus is conceptually different from basic IPC:

* IPC provides the communication mechanism.
* Knowledge Bus provides an AI-oriented shared information mechanism.

---

# 25. Checkpoint Manager

The Checkpoint Manager saves the simulated state of an AI agent.

```text
AI Agent
   │
   ▼
Checkpoint Manager
   │
   ▼
Saved State
```

The saved state may include:

* Program Counter
* Registers
* Process State
* Memory Mapping
* Current Task
* Agent State

---

# 26. Failure Recovery Architecture

When an AI agent fails:

```text
AI Agent
   │
   ▼
Failure
   │
   ▼
Error Manager
   │
   ▼
Checkpoint Manager
   │
   ▼
Restore State
   │
   ▼
Process Manager
   │
   ▼
Ready Queue
   │
   ▼
Scheduler
   │
   ▼
Agent Resumes
```

This demonstrates interaction between multiple OS components rather than treating recovery as an isolated AI feature.

---

# 27. Frontend Architecture

The frontend is responsible for visualization and user interaction.

```text
React Frontend
│
├── Dashboard
├── CPU Monitor
├── Process Monitor
├── Memory Monitor
├── Scheduler View
├── Interrupt Log
├── IPC Monitor
├── I/O Monitor
├── AI Agent Monitor
└── Event Log
```

The frontend does not directly modify internal OS state.

It sends commands to the C++ engine.

---

# 28. Backend–Frontend Communication

The architecture uses two communication mechanisms.

```text
                   React
                     │
          ┌──────────┴──────────┐
          │                     │
        REST                WebSocket
          │                     │
          ▼                     ▼
       Commands            Live Events
          │                     │
          └──────────┬──────────┘
                     ▼
                C++ Engine
```

### REST

Used for operations such as:

* Start simulation
* Stop simulation
* Create process
* Create agent
* Select scheduling algorithm
* Load scenario

### WebSocket

Used for real-time events such as:

* CPU state changes
* Context switches
* Page faults
* Interrupts
* Process state changes
* IPC messages
* Agent failure
* Recovery events

---

# 29. Event-Based Communication

The C++ engine shall produce simulation events.

Example:

```text
EVENT:
{
    "type": "PAGE_FAULT",
    "pid": 4,
    "page": 3
}
```

The frontend receives the event and updates the appropriate component.

Other possible events:

```text
PROCESS_CREATED
PROCESS_STARTED
CONTEXT_SWITCH
PAGE_FAULT
PAGE_LOADED
INTERRUPT_GENERATED
INTERRUPT_SERVICED
IPC_MESSAGE
IO_REQUEST
IO_COMPLETED
AGENT_FAILED
AGENT_RECOVERED
```

The exact event schema will be defined during implementation.

---

# 30. Complete System Data Flow

A typical process execution may follow:

```text
User
 │
 ▼
React Dashboard
 │
 │ Start Simulation
 ▼
C++ Engine
 │
 ▼
Process Manager
 │
 ▼
Scheduler
 │
 ▼
CPU
 │
 ▼
Memory Manager
 │
 ├── Page Present ─────► Continue
 │
 └── Page Missing
          │
          ▼
      Page Fault
          │
          ▼
   Interrupt Manager
          │
          ▼
   Memory Manager
          │
          ▼
    Page Replacement
          │
          ▼
        CPU
```

---

# 31. AI Agent Data Flow

An AI agent may follow:

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
Scheduler
   │
   ▼
CPU
   │
   ├──────────────► Memory Manager
   │
   ├──────────────► I/O Manager
   │
   ├──────────────► IPC Manager
   │
   └──────────────► Permission Manager
                         │
                         ▼
                    Tool Request
```

The agent remains subject to OS resource and process management.

---

# 32. Major Component Interaction

```text
                         ┌───────────────┐
                         │   Scheduler   │
                         └───────┬───────┘
                                 │
                                 ▼
┌──────────────┐         ┌───────────────┐
│   Process    │◄───────►│      CPU      │
│   Manager    │         └───────┬───────┘
└──────┬───────┘                 │
       │                         │
       ▼                         ▼
┌──────────────┐         ┌───────────────┐
│    Memory    │         │   Interrupt   │
│   Manager    │◄───────►│    Manager    │
└──────────────┘         └───────┬───────┘
                                  │
       ┌──────────────────────────┼──────────────────┐
       │                          │                  │
       ▼                          ▼                  ▼
┌──────────────┐          ┌──────────────┐   ┌──────────────┐
│     IPC      │          │      I/O     │   │     Error    │
│   Manager    │          │   Manager    │   │   Manager    │
└──────┬───────┘          └──────────────┘   └──────────────┘
       │
       ▼
┌──────────────────────────────────────────────────────┐
│                    AI Layer                          │
│                                                      │
│ Agent Manager │ AI Scheduler │ Permissions │ etc.   │
└──────────────────────────────────────────────────────┘
```

---

# 33. Architectural Principles

AIOS shall follow these principles:

### 33.1 Separation of Concerns

Each component shall have a clearly defined responsibility.

### 33.2 OS First

The core OS mechanisms shall operate independently of the AI layer.

### 33.3 AI as an Extension

AI agents shall use OS services rather than bypassing them.

### 33.4 Frontend Independence

The simulator shall be capable of running independently of the React frontend.

### 33.5 Explainability

Scheduling, memory, interrupt, and recovery decisions should produce understandable events and logs.

### 33.6 Deterministic Simulation

Core demonstration scenarios should be reproducible.

### 33.7 Modular Development

Each team member should be able to work on a defined subsystem with minimal coupling.

---

# 34. Architectural Dependency Rules

The following dependency direction should be maintained:

```text
Frontend
   ↓
Communication Layer
   ↓
OS / Simulation Layer
   ↓
Hardware Simulation
```

The AI layer interacts with the OS layer but should not directly control simulated hardware.

```text
AI Layer
   ↓
OS Services
   ↓
Simulated Hardware
```

For example, an AI agent requiring memory should request memory through the Memory Manager rather than directly modifying RAM.

---

# 35. Final Architecture

The complete conceptual architecture is:

```text
                         USER
                          │
                          ▼
                ┌───────────────────┐
                │  React Dashboard  │
                │ TypeScript/Tailwind│
                └─────────┬─────────┘
                          │
                    REST / WebSocket
                          │
                          ▼
                ┌───────────────────┐
                │    C++ AIOS       │
                │      Engine       │
                ├───────────────────┤
                │ Process Manager   │
                │ Scheduler         │
                │ Memory Manager    │
                │ Interrupt Manager │
                │ System Calls     │
                │ Synchronization   │
                │ IPC               │
                │ I/O               │
                │ Error Manager     │
                ├───────────────────┤
                │ AI Agent Manager  │
                │ AI Scheduler      │
                │ Resource Manager  │
                │ Permission Manager│
                │ Knowledge Bus     │
                │ Checkpoint Manager│
                └─────────┬─────────┘
                          │
                          ▼
                ┌───────────────────┐
                │ Simulated Hardware│
                ├───────────────────┤
                │ CPU               │
                │ Registers         │
                │ RAM               │
                │ Pages / Frames    │
                │ Page Tables       │
                │ Swap              │
                │ I/O Devices       │
                └───────────────────┘
```

This architecture provides the foundation for the detailed subsystem designs that follow.
