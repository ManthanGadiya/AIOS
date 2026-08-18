# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 2. Requirements

## 1. Purpose

This document defines the functional and non-functional requirements of AIOS.

The requirements are derived from the three stages of the Multiprogramming Operating System project and the additional AI Agent Management Layer defined in the project overview.

The purpose of this document is to establish a fixed scope for implementation, testing, demonstration, and viva preparation.

---

# 2. Requirement Categories

AIOS requirements are divided into four major categories:

```text
AIOS Requirements
│
├── Stage I Requirements
│   ├── CPU / Machine Simulation
│   └── Supervisor Calls through Interrupts
│
├── Stage II Requirements
│   ├── Paging
│   ├── Error Handling
│   ├── Interrupt Generation & Servicing
│   └── Process Data Structure
│
├── Stage III Requirements
│   ├── Multiprogramming
│   ├── Virtual Memory
│   ├── Scheduling
│   ├── Synchronization
│   ├── IPC
│   └── I/O, Spooling & Buffering
│
└── AI Extension Requirements
    ├── AI Agent Management
    ├── AI Adaptive Scheduling
    ├── Resource Quotas
    ├── Tool Permissions
    ├── Agent Communication
    └── Checkpoint & Recovery
```

---

# 3. Stage I Requirements

## 3.1 CPU / Machine Simulation

The system shall provide a simulated CPU capable of executing a defined set of machine instructions.

The simulated CPU shall contain:

```text
CPU
├── PC      Program Counter
├── IR      Instruction Register
├── ACC     Accumulator
├── MAR     Memory Address Register
├── MBR     Memory Buffer Register
└── FLAGS   Status Flags
```

### Functional Requirements

The CPU simulator shall:

* Maintain CPU register values.
* Fetch instructions from simulated memory.
* Decode instructions.
* Execute instructions.
* Update registers after execution.
* Maintain the Program Counter.
* Detect invalid instructions.
* Support normal process termination using a HALT instruction.
* Generate appropriate events when an instruction requires OS intervention.

### Initial Instruction Set

The initial instruction set shall contain a small set of instructions sufficient for demonstrating OS functionality.

Proposed instructions:

```text
LOAD
STORE
ADD
SUB
MUL
DIV
JMP
JZ
READ
WRITE
SYSCALL
HALT
```

The instruction set may be extended if required during implementation.

---

## 3.2 Supervisor Calls Through Interrupts

Processes shall be able to request operating system services through supervisor calls.

A supervisor call shall transfer control from the executing process to the OS.

Example:

```text
Process
   │
   │ SYSCALL
   ▼
Interrupt Generation
   │
   ▼
Kernel / OS Handler
   │
   ▼
Requested Service
   │
   ▼
Return to Process
```

The system shall support simulated OS services such as:

* Input
* Output
* Memory request
* Process-related requests
* I/O requests

The exact system call interface will be defined in `05_System_Calls_Interrupts.md`.

---

# 4. Stage II Requirements

## 4.1 Paging

The system shall implement paging-based memory management.

The memory manager shall represent:

```text
Logical Memory
      │
      ▼
    Pages
      │
      ▼
 Page Table
      │
      ▼
   Frames
      │
      ▼
 Physical Memory
```

### Functional Requirements

The system shall:

* Divide logical memory into fixed-size pages.
* Divide physical memory into fixed-size frames.
* Maintain a page table for each process.
* Map pages to physical frames.
* Perform logical-to-physical address translation.
* Track page presence.
* Detect pages that are not currently loaded into memory.
* Generate a page fault when required.

---

## 4.2 Error Handling

The OS shall detect and handle errors occurring during simulation.

Initial error conditions shall include:

```text
Invalid Instruction
Invalid Memory Address
Divide by Zero
Invalid System Call
Page Fault
Invalid Process Operation
```

The error manager shall:

1. Detect the error.
2. Identify the affected process.
3. Generate an appropriate OS event/interrupt where applicable.
4. Record the error.
5. Apply the appropriate recovery or termination policy.

---

## 4.3 Interrupt Generation and Servicing

AIOS shall support simulated interrupts.

Possible interrupt sources include:

```text
Timer
System Call
Page Fault
I/O Completion
Process/Event Completion
Error
```

The Interrupt Manager shall:

* Detect interrupt events.
* Identify the interrupt type.
* Save required process execution state.
* Transfer control to the appropriate interrupt service routine.
* Execute the service routine.
* Restore execution state where applicable.
* Resume or reschedule the affected process.

---

## 4.4 Process Data Structure

Each process shall be represented using a Process Control Block (PCB).

The PCB shall contain the information required to manage process execution.

Initial PCB fields:

```text
PCB
├── PID
├── Process Name
├── Process State
├── Program Counter
├── Registers
├── Priority
├── Memory Information
├── Page Table Reference
├── CPU Time
├── Creation Time
└── Scheduling Information
```

The exact structure will be finalized during the Process Management design.

---

# 5. Stage III Requirements

## 5.1 Multiprogramming

AIOS shall support multiple processes residing in the system and competing for system resources.

The initial demonstration shall contain:

```text
3 Normal Processes
+
2 AI Agents
```

The processes shall transition between states such as:

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
TERMINATED
```

The system shall allow the scheduler to select among multiple ready processes.

---

# 5.2 Virtual Memory

AIOS shall implement virtual memory using paging.

When a required page is not available in physical memory:

```text
Process
   ↓
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
Page Replacement
   ↓
Page Loaded
   ↓
Process Resumes
```

The system shall support a simulated swap area.

The page replacement algorithm shall be defined during the memory management design.

Candidate algorithms include:

* FIFO
* LRU

The final algorithm selection will be documented in `07_Paging_Virtual_Memory.md`.

---

# 5.3 Process Scheduling

AIOS shall implement multiple CPU scheduling algorithms.

Required algorithms:

### FCFS

First Come First Serve.

### Round Robin

Processes receive a fixed time quantum.

### Priority Scheduling

Processes are selected according to priority.

### AI Adaptive Scheduling

The AI scheduler shall use predefined intelligent heuristics to calculate scheduling decisions.

Potential factors include:

```text
Priority
Waiting Time
CPU Usage
Memory Usage
Task Urgency
Resource Requirements
```

The AI scheduler shall not require an external machine-learning model.

---

# 5.4 Process Synchronization

AIOS shall provide synchronization mechanisms for processes accessing shared resources.

The implementation shall support:

* Mutex
* Semaphore

The system shall be capable of demonstrating a race-condition prevention scenario.

Example:

```text
Process A ──┐
            ├── Shared Resource
Process B ──┘
```

Synchronization shall ensure that conflicting operations are controlled.

---

# 5.5 Inter-Process Communication

AIOS shall support communication between processes.

Initial IPC mechanisms:

```text
Message Queue
Shared Memory
Pipe
```

The exact mechanisms implemented in the first working version may be prioritized based on development time.

AI agents shall also use IPC for agent-to-agent communication.

---

# 5.6 I/O Handling

The system shall simulate I/O operations.

Possible simulated devices include:

```text
Disk
Keyboard
Printer
Network
```

I/O operations shall be represented as OS-managed events rather than real hardware operations.

The I/O Manager shall:

* Receive I/O requests.
* Place requests in appropriate queues.
* Block the requesting process when necessary.
* Generate an I/O completion event.
* Wake or reschedule the waiting process.

---

# 5.7 Buffering

AIOS shall implement simulated buffering.

A buffer shall temporarily hold data between a producer and consumer operating at different speeds.

Example:

```text
Process
   ↓
Output Buffer
   ↓
I/O Device
```

The simulator shall visualize buffer state where appropriate.

---

# 5.8 Spooling

AIOS shall implement simulated spooling using a queue of pending I/O requests.

Example:

```text
P1 ──┐
P2 ──┼──► Spool Queue ──► Printer
P3 ──┘
```

The spooler shall:

* Accept requests.
* Store them in a queue.
* Process requests according to the defined policy.
* Update request status.

---

# 6. AI Extension Requirements

The AI layer is an extension to the required operating system functionality.

It shall not replace the core OS mechanisms.

---

## 6.1 AI Agent Manager

The AI Agent Manager shall create, track, and terminate simulated AI agents.

Initial agents:

```text
Research Agent
Coding Agent
```

Additional agents may be added if required.

Each AI agent shall have:

```text
Agent ID
Agent Type
PID
State
Priority
Memory Requirement
CPU Requirement
Token Budget
Tool Permissions
Checkpoint
```

---

# 6.2 Simulated Agent Behaviour

AI agents shall simulate workload behaviour rather than perform real autonomous reasoning.

Example:

```text
Research Agent
    ↓
CPU Work
    ↓
I/O Request
    ↓
IPC Message
    ↓
Task Completion
```

The agent behaviour shall be deterministic or controllable enough to make demonstrations reproducible.

---

# 6.3 AI Adaptive Scheduler

The AI Scheduler shall operate as an intelligent scheduling policy.

It shall evaluate process/agent characteristics and calculate a scheduling preference.

Possible inputs:

```text
Base Priority
Waiting Time
CPU Usage
Memory Usage
Task Urgency
Resource Consumption
```

The scheduler shall produce an explainable decision.

Example:

```text
AI Scheduler Decision

Selected: Research Agent

Reason:
- High waiting time
- Moderate CPU usage
- High task urgency
```

The exact scoring formula will be defined in the Scheduling document.

---

# 6.4 Resource Quotas

AIOS shall allow simulated resource limits to be assigned to AI agents.

Possible quotas:

```text
CPU Time
Memory
I/O Requests
Token Budget
```

The Resource Manager shall track resource consumption.

If an agent exceeds a configured limit, the OS shall apply a defined policy such as:

* Block
* Deprioritize
* Reject request
* Terminate

The exact policy will be finalized during implementation.

---

# 6.5 Tool Permissions

AI agents shall have simulated permissions for accessing tools/resources.

Example:

```text
Research Agent
├── Internet     ALLOWED
├── File System  ALLOWED
├── Python       ALLOWED
└── Camera       DENIED
```

The Permission Manager shall check an agent's permission before allowing a simulated tool request.

---

# 6.6 Agent-to-Agent Communication

AIOS shall allow AI agents to communicate through the IPC layer.

Example:

```text
Research Agent
      │
      │ Message
      ▼
 Knowledge Bus
      │
      ▼
Coding Agent
```

The Knowledge Bus is an AI-oriented extension and shall be implemented on top of the project's communication mechanisms.

---

# 6.7 Checkpointing

AIOS shall support simulated checkpoints for AI agents.

A checkpoint shall store sufficient information to restore the agent's simulated execution state.

At minimum, this may include:

```text
Program Counter
Register State
Process State
Memory Mapping
Current Task
Agent State
```

---

# 6.8 Failure Recovery

The system shall support intentional failure of an AI agent.

The recovery sequence shall be:

```text
Running Agent
      ↓
Checkpoint
      ↓
Simulated Failure
      ↓
Error Handler
      ↓
Checkpoint Manager
      ↓
Restore State
      ↓
Resume / Restart
```

The recovery result shall be visible in the dashboard.

---

# 7. Frontend Requirements

The React dashboard shall provide visualization and control of the OS simulator.

The dashboard shall display, where applicable:

### CPU

```text
PC
IR
ACC
MAR
MBR
FLAGS
```

### Processes

```text
PID
Name
State
Priority
CPU Time
```

### Memory

```text
RAM
Frames
Pages
Page Table
Swap
```

### Scheduler

```text
Ready Queue
Running Process
Waiting Processes
Scheduling Algorithm
Scheduling Metrics
```

### Interrupts

```text
Interrupt Type
Source
Status
Timestamp
```

### IPC

```text
Sender
Receiver
Message
Status
```

### AI Agents

```text
Agent
State
Resource Usage
Token Budget
Permissions
Checkpoint
```

---

# 8. Backend Requirements

The C++ engine shall contain the actual simulation logic.

The backend shall be responsible for:

* CPU execution
* Process creation
* Process state transitions
* Scheduling
* Memory management
* Paging
* Virtual memory
* Interrupts
* System calls
* Synchronization
* IPC
* I/O
* Error handling
* AI agent management
* AI scheduling
* Checkpointing

The frontend shall not directly implement OS logic.

---

# 9. Communication Requirements

The frontend and backend shall communicate using:

```text
React Frontend
      │
      ├── REST
      │
      └── WebSocket
              │
              ▼
       C++ OS Simulator
```

### REST

Used for commands/configuration where real-time updates are not required.

### WebSocket

Used for real-time simulation events such as:

* Context switches
* Page faults
* Interrupts
* Process state changes
* IPC messages
* Agent failures
* Recovery events

---

# 10. Non-Functional Requirements

## 10.1 Modularity

Each OS component shall be implemented as an independent module with well-defined interfaces.

## 10.2 Explainability

Important OS and AI decisions shall be explainable in the GUI and logs.

For example:

```text
Context Switch:
P1 → P2

Reason:
Time Quantum Expired
```

or:

```text
Page Fault:
Process P2
Page: 4

Reason:
Page not present in RAM
```

## 10.3 Deterministic Demonstration

The primary demonstration scenarios should produce predictable results so they can be reliably reproduced during the viva.

## 10.4 Maintainability

The codebase shall be organized so that individual team members can work on separate modules without unnecessarily modifying unrelated components.

## 10.5 Performance

The simulator should support multiple processes and AI agents without noticeable UI freezing during normal demonstrations.

## 10.6 Visualization

Important state changes should be visible through the dashboard.

---

# 11. Initial Demonstration Requirements

The system shall support the following demonstration scenario:

```text
1. Create 3 normal processes
2. Create 2 AI agents
3. Load processes/agents into memory
4. Display process states
5. Start the scheduler
6. Demonstrate context switching
7. Generate a page fault
8. Service the page fault interrupt
9. Demonstrate IPC between two AI agents
10. Create an agent checkpoint
11. Simulate an agent crash
12. Recover the agent from the checkpoint
13. Execute FCFS
14. Execute Round Robin
15. Execute Priority Scheduling
16. Execute AI Adaptive Scheduling
17. Compare scheduling results
```

---

# 12. Requirement Priority

Because the project has a two-month development period, requirements are prioritized.

## Priority 1 — Mandatory OS Core

```text
CPU Simulation
System Calls
Interrupts
Process Management
Paging
Page Faults
Virtual Memory
Multiprogramming
Scheduling
Synchronization
IPC
I/O
Buffering
Spooling
Error Handling
```

## Priority 2 — Required AI Extension

```text
AI Agent Simulation
AI Adaptive Scheduler
Agent Resource Quotas
Agent Permissions
Agent IPC
Checkpointing
Failure Recovery
```

## Priority 3 — Visualization Enhancements

```text
Advanced Charts
Detailed Animations
Advanced Metrics
Additional Agent Types
```

Priority 3 features shall not delay completion of Priority 1 or Priority 2 requirements.

---

# 13. Requirements Traceability

The implementation shall maintain a direct relationship between the academic requirements and project modules.

| Academic Requirement     | AIOS Component              |
| ------------------------ | --------------------------- |
| CPU / Machine Simulation | CPU Simulator               |
| Supervisor Call          | System Call Manager         |
| Interrupt                | Interrupt Manager           |
| Paging                   | Memory Manager              |
| Error Handling           | Error Manager               |
| Process Data Structure   | Process Manager / PCB       |
| Multiprogramming         | Process Manager + Scheduler |
| Virtual Memory           | Memory Manager              |
| Process Scheduling       | Scheduler                   |
| Synchronization          | Synchronization Manager     |
| IPC                      | IPC Manager                 |
| I/O Handling             | I/O Manager                 |
| Spooling                 | Spooler                     |
| Buffering                | Buffer Manager              |
| AI Extension             | AI Agent Manager            |
| AI Scheduling            | AI Adaptive Scheduler       |
| Agent Recovery           | Checkpoint Manager          |

---

# 14. Requirement Completion Rule

A feature shall not be considered complete merely because the code exists.

A requirement is considered implemented only when:

```text
Implementation
      +
Test
      +
Observable Result
      +
Documentation
```

For example, paging is complete only when the simulator can demonstrate:

```text
Page Table
   ↓
Page Not Present
   ↓
Page Fault
   ↓
Interrupt
   ↓
Page Replacement
   ↓
Page Loaded
   ↓
Process Resumes
```

---

# 15. Scope Control

The following principle shall be followed throughout development:

> **Core OS correctness has higher priority than AI complexity or UI complexity.**

If development time becomes limited, features shall be reduced in this order:

```text
Advanced UI
      ↓
Optional AI Features
      ↓
Optional IPC Features
      ↓
Optional Scheduling Enhancements
```

The mandatory OS requirements shall not be removed merely to add additional AI functionality.

---

# 16. Definition of Done

The project is considered ready for final demonstration when:

* All mandatory Stage I requirements work.
* All mandatory Stage II requirements work.
* All mandatory Stage III requirements work.
* At least two AI agents can be simulated.
* AI Adaptive Scheduling works.
* Agent checkpoint and recovery work.
* The complete demonstration scenario can be executed.
* The frontend displays the important simulator states.
* Tests exist for the major OS modules.
* The team can explain each component during the viva.
