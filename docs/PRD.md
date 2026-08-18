# AIOS — Product Requirements Document (PRD)

## AI-Powered Multiprogramming Operating System Simulator

**Version:** 1.0  
**Project Type:** Operating System Simulator  
**Platform:** Desktop/Web-based Simulator  
**Development Duration:** 2 Months  
**Primary Language:** C++  
**Frontend:** React + TypeScript  
**Status:** Development Planning  

---

# 1. Product Overview

## 1.1 Product Name

AIOS — AI-Powered Multiprogramming Operating System Simulator

---

## 1.2 Product Vision

AIOS is a simulated multiprogramming operating system that demonstrates fundamental Operating System concepts through a visual and interactive environment.

The system combines:

- CPU / Machine Simulation
- Process Management
- Multiprogramming
- Paging
- Virtual Memory
- Interrupt Handling
- Process Scheduling
- Synchronization
- Inter-Process Communication
- I/O Handling
- Buffering
- Spooling
- Simulated AI Agents
- AI-based scheduling heuristics
- Agent checkpointing
- Agent failure and recovery

The objective is to provide a complete OS simulation where the internal behavior of the operating system can be observed through a graphical dashboard.

---

# 2. Problem Statement

Traditional Operating System concepts are difficult to understand when represented only through theoretical diagrams or isolated programs.

A student may understand:

- Process Scheduling
- Paging
- Interrupts
- IPC
- Synchronization

individually, but it can be difficult to visualize how these components interact inside an operating system.

AIOS addresses this problem by creating an integrated simulator where OS operations can be observed in real time.

For example:


Process Execution
      ↓
Memory Access
      ↓
Page Fault
      ↓
Interrupt
      ↓
Memory Handling
      ↓
Process Resumes
      ↓
Scheduler
      ↓
Context Switch

The system therefore demonstrates not only individual OS concepts but also their interaction.

---

# 3. Product Goal

The primary goal is to develop a working multiprogramming OS simulator that:

1. Simulates a CPU and machine.
2. Simulates process creation and management.
3. Supports multiple processes executing concurrently through multiprogramming.
4. Simulates paging and virtual memory.
5. Generates and handles interrupts.
6. Implements process scheduling.
7. Demonstrates synchronization.
8. Demonstrates IPC.
9. Simulates I/O, buffering, and spooling.
10. Introduces AI agents as OS-managed workloads.
11. Provides an explainable AI Adaptive Scheduler.
12. Demonstrates AI-agent checkpointing and recovery.
13. Provides a real-time graphical dashboard.
14. Supports predefined demonstration scenarios.

---

# 4. Project Scope

## 4.1 In Scope

The following functionality is included in AIOS.

### Stage I

* CPU / Machine Simulation
* CPU Registers
* Instruction Execution
* Supervisor Calls
* System Call through Interrupt

### Stage II

* Paging
* Page Tables
* Physical Frames
* Virtual Memory
* Page Faults
* Error Handling
* Interrupt Generation
* Interrupt Servicing
* Process Data Structure / PCB

### Stage III

* Multiprogramming
* Virtual Memory
* Process Scheduling
* Synchronization
* Inter-Process Communication
* I/O Handling
* Buffering
* Spooling

### AI Layer

* Simulated AI Agents
* AI Agent Manager
* AI Adaptive Scheduler
* Agent Resource Quotas
* Tool Permissions
* Agent-to-Agent IPC
* Agent Checkpointing
* Simulated Agent Failure
* Agent Recovery

### Frontend

* Dashboard
* Process Monitoring
* AI Agent Monitoring
* CPU Monitoring
* Memory Visualization
* Page Table Visualization
* Scheduler Visualization
* Interrupt Monitoring
* IPC Monitoring
* Synchronization Monitoring
* I/O Monitoring
* Event Logs
* Demo Scenario Controls

---

# 5. Out of Scope

The following are explicitly outside the project's scope:

* Building a real operating system kernel.
* Booting on physical hardware.
* Real CPU scheduling of host-machine processes.
* Real GPU management.
* Running real autonomous AI agents.
* Training an AI/ML model.
* Calling an external LLM API as a core requirement.
* Building a production-grade distributed AI-agent platform.
* Real hardware device drivers.
* Real disk management.
* Production-level security.
* Production-level virtualization.

AI agents are simulated workloads.

---

# 6. Target Users

## Primary Users

### Students

Students can use AIOS to understand OS concepts through simulation.

### Faculty / Evaluators

Faculty can use the simulator to evaluate:

* OS concepts
* Implementation
* Integration
* Scheduling
* Memory Management
* IPC
* Synchronization
* AI-agent integration

### Project Team

The team will use AIOS to demonstrate the complete implementation during project reviews and viva.

---

# 7. Core Product Architecture

The system consists of two major layers:

```text
                    AIOS
                     │
          ┌──────────┴──────────┐
          │                     │
     Core OS Layer         AI Agent Layer
          │                     │
          │              AI Agent Manager
          │                     │
          ├──────────────┬──────┤
          │              │
          ▼              ▼
      Scheduler        Agents
          │              │
          ├──────┬───────┤
          │      │
          ▼      ▼
       Memory    IPC
          │
          ▼
         CPU
```

The GUI exists above the OS engine.

```text
React Frontend
      │
      │ REST / WebSocket
      ▼
C++ OS Engine
```

---

# 8. System Components

## 8.1 CPU / Machine Simulator

The simulated CPU contains:

* PC
* IR
* ACC
* MAR
* MBR
* FLAGS

The CPU performs the instruction cycle:

```text
FETCH
  ↓
DECODE
  ↓
EXECUTE
  ↓
FETCH
```

---

## 8.2 Memory Manager

Responsible for:

* RAM
* Pages
* Frames
* Page Tables
* Address Translation
* Page Faults
* Virtual Memory
* Swap / Page Replacement

---

## 8.3 Process Manager

Responsible for:

* Process Creation
* Process Termination
* Process State
* PCB
* CPU Context
* Process Memory Information

Supported states:

```text
NEW
READY
RUNNING
WAITING
TERMINATED
FAILED
```

---

## 8.4 Scheduler

Supported algorithms:

```text
FCFS
Round Robin
Priority
AI Adaptive
```

The Scheduler manages the Ready Queue and determines which workload receives CPU time.

---

## 8.5 Interrupt Manager

Responsible for:

* Interrupt Generation
* Interrupt Recognition
* Interrupt Servicing
* Interrupt Handler Selection

Supported interrupt categories include:

```text
Timer Interrupt
I/O Interrupt
Page Fault
System Call Interrupt
IPC Event
```

---

## 8.6 Synchronization Manager

Responsible for simulated synchronization mechanisms such as:

* Mutex
* Critical Sections
* Waiting Processes

The system shall demonstrate race conditions and their prevention.

---

## 8.7 IPC Manager

Responsible for:

* Message Sending
* Message Receiving
* Message Queues
* Blocking Communication
* Agent-to-Agent Communication

---

## 8.8 I/O Manager

Responsible for simulated devices such as:

* Disk
* Keyboard
* Network
* Printer

The I/O subsystem shall demonstrate:

* I/O Requests
* Device Queues
* Blocking
* I/O Completion
* I/O Interrupts

---

## 8.9 Buffering and Spooling

### Buffering

The system shall simulate temporary storage during I/O operations.

### Spooling

The system shall simulate queued jobs for a shared device such as a printer.

Example:

```text
P1 ──┐
P2 ──┤
A1 ──┼──► Printer Spool Queue ──► Printer
A2 ──┘
```

---

# 9. AI Agent Layer

## 9.1 Concept

AI agents are simulated OS-managed workloads.

Example:

```text
Research Agent
      ↓
Process Manager
      ↓
Scheduler
      ↓
CPU
```

An AI agent is not allowed to bypass the OS.

---

# 10. Initial AI Agents

The initial system shall provide two simulated AI agents:

```text
A1 → Research Agent
A2 → Coding Agent
```

---

# 11. Research Agent

The Research Agent represents an information-processing workload.

It can simulate:

* Data processing
* Research tasks
* Search requests
* Result generation
* IPC communication

---

# 12. Coding Agent

The Coding Agent represents a coding-oriented workload.

It can simulate:

* Code-generation tasks
* Processing requests
* Receiving research results
* Sending task acknowledgements

---

# 13. AI Agent Metadata

Each agent shall maintain:

```text
Agent ID
Agent Name
PID
Agent Type
Priority
Task
Token Budget
Resource Quota
Tool Permissions
Checkpoint
Failure State
```

---

# 14. AI Adaptive Scheduler

## 14.1 Purpose

The AI Adaptive Scheduler is the project's intelligent scheduling component.

It does not require an external AI model.

It uses deterministic rules and heuristics.

---

## 14.2 Scheduling Factors

The scheduler may consider:

```text
Priority
Waiting Time
Urgency
Resource Usage
```

A conceptual score is:

```text
Score =
    Priority Contribution
  + Waiting Time Contribution
  + Urgency Contribution
  - Resource Cost
```

The exact weights shall be configurable.

---

## 14.3 Explainability

The scheduler must explain its decision.

Example:

```text
Selected:
Research Agent

Score:
82.5

Reasons:
+ High waiting time
+ High urgency
+ High priority
- Moderate resource usage
```

---

# 15. Agent Resource Management

AI agents may have simulated resource quotas.

Examples:

```text
CPU Quota
Memory Quota
Token Budget
I/O Limit
```

Example:

```text
Research Agent

CPU Quota:
20 cycles

Memory:
8 pages

Token Budget:
1000
```

---

# 16. Tool Permission System

Agents may have simulated tool permissions.

Example:

```text
Research Agent:

READ_DATA → Allowed
SEARCH → Allowed
WRITE_CODE → Denied
```

If an unauthorized request occurs:

```text
Agent
  ↓
Tool Request
  ↓
Permission Check
  ↓
Permission Denied
```

---

# 17. Agent IPC

Agents communicate using the OS IPC subsystem.

Example:

```text
Research Agent
      │
      │ RESEARCH_RESULT
      ▼
IPC Manager
      │
      ▼
Coding Agent
```

Agents must not directly manipulate one another's memory.

---

# 18. Checkpointing

The system shall allow an AI agent to create a checkpoint.

A checkpoint represents a saved simulated execution state.

It may contain:

```text
CPU Context
Process State
Memory References
Agent Metadata
Execution Progress
```

---

# 19. Agent Failure and Recovery

The system shall support simulated agent failure.

Example:

```text
RUNNING
   ↓
SIMULATED CRASH
   ↓
FAILED
   ↓
CHECKPOINT RESTORE
   ↓
READY
   ↓
SCHEDULER
   ↓
RUNNING
```

The purpose is to demonstrate fault tolerance at the simulated OS level.

---

# 20. Initial Workload

The standard demonstration workload shall contain:

```text
P1 → Text Editor
P2 → Compiler
P3 → Music Player

A1 → Research Agent
A2 → Coding Agent
```

This workload provides enough entities to demonstrate multiprogramming, scheduling, memory management, IPC, and AI-agent behavior.

---

# 21. CPU Requirements

The CPU simulator shall:

1. Maintain CPU registers.
2. Fetch instructions.
3. Decode instructions.
4. Execute instructions.
5. Maintain CPU state.
6. Support context switching.
7. Support interrupt handling.

---

# 22. Memory Requirements

The memory subsystem shall:

1. Represent physical memory.
2. Divide memory into frames.
3. Represent process memory using pages.
4. Maintain page tables.
5. Translate logical addresses.
6. Detect page faults.
7. Load missing pages.
8. Support virtual memory.
9. Support page replacement/swap behavior where required.

---

# 23. Process Requirements

The Process Manager shall:

1. Create processes.
2. Assign PIDs.
3. Create PCBs.
4. Maintain process states.
5. Allocate memory.
6. Place processes in the Ready Queue.
7. Save CPU context.
8. Restore CPU context.
9. Terminate processes.

---

# 24. Scheduling Requirements

The Scheduler shall:

1. Maintain the Ready Queue.
2. Select the next workload.
3. Support multiple scheduling algorithms.
4. Perform context switching.
5. Track scheduling metrics.
6. Support AI Adaptive scheduling.
7. Explain AI scheduling decisions.

---

# 25. Synchronization Requirements

The synchronization subsystem shall:

1. Provide mutex functionality.
2. Track lock ownership.
3. Maintain waiting processes.
4. Protect critical sections.
5. Demonstrate race conditions.
6. Demonstrate race-condition prevention.

---

# 26. IPC Requirements

The IPC subsystem shall:

1. Create message queues.
2. Send messages.
3. Receive messages.
4. Support blocking receive behavior.
5. Wake waiting processes when messages arrive.
6. Support communication between AI agents.

---

# 27. I/O Requirements

The I/O subsystem shall:

1. Represent simulated devices.
2. Maintain device states.
3. Maintain device queues.
4. Process I/O requests.
5. Block processes waiting for I/O.
6. Generate I/O completion interrupts.

---

# 28. GUI Requirements

The GUI shall provide:

```text
Dashboard
Processes
AI Agents
CPU
Memory
I/O Devices
IPC
Scheduler
Interrupts
Logs
Settings
```

---

# 29. Dashboard Requirements

The Dashboard shall display:

```text
Total Processes
AI Agents
CPU Usage
Memory Usage
Page Faults
Interrupts
```

It shall also display:

```text
Process List
AI Agent List
CPU State
Memory
Page Table
Ready Queue
Event Log
IPC Queue
I/O Devices
```

---

# 30. GUI Controls

The Dashboard shall provide:

```text
Start
Pause
Stop
Reset
```

It shall also provide:

```text
Scheduler Selection
Clock / Simulation Speed
Demo Scenario Selection
```

---

# 31. GUI Live Updates

The frontend shall receive live simulation events.

Architecture:

```text
C++ OS Engine
      │
      │ WebSocket
      ▼
React Frontend
```

Example event:

```json
{
  "type": "PROCESS_STATE_CHANGED",
  "pid": 4,
  "old_state": "READY",
  "new_state": "RUNNING"
}
```

---

# 32. Backend Communication

REST/HTTP shall be used for:

```text
Commands
Configuration
Initial State
Scenario Setup
```

WebSocket shall be used for:

```text
CPU Updates
Process Updates
Memory Events
Interrupts
IPC Events
I/O Events
Scheduler Events
AI Agent Events
```

---

# 33. Event System

Important events include:

```text
PROCESS_CREATED
PROCESS_TERMINATED
CONTEXT_SWITCH
PAGE_FAULT
PAGE_LOADED
INTERRUPT_GENERATED
INTERRUPT_HANDLED
IPC_SEND
IPC_RECEIVE
IO_REQUEST
IO_COMPLETE
MUTEX_LOCK
MUTEX_UNLOCK
AGENT_CREATED
AGENT_FAILURE
CHECKPOINT_CREATED
AGENT_RECOVERED
```

---

# 34. Demo Scenarios

The system shall provide predefined scenarios.

### Scenario 1

Basic CPU / Machine Simulation

### Scenario 2

Supervisor Call / System Call

### Scenario 3

Multiprogramming

### Scenario 4

Process Scheduling

### Scenario 5

Paging / Page Fault

### Scenario 6

Interrupt Handling

### Scenario 7

Synchronization

### Scenario 8

IPC

### Scenario 9

I/O

### Scenario 10

Buffering

### Scenario 11

Spooling

### Scenario 12

AI Adaptive Scheduling

### Scenario 13

AI Agent Failure and Recovery

### Scenario 14

Full Integrated OS Demonstration

### Scenario 15

Scheduling Algorithm Comparison

---

# 35. Full Demonstration Flow

The primary final demonstration shall follow:

```text
Create 3 Processes + 2 AI Agents
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
Create Checkpoint
          ↓
Simulate Agent Crash
          ↓
Recover From Checkpoint
          ↓
Resume Execution
          ↓
Compare Scheduling Algorithms
```

---

# 36. Testing Requirements

The system shall be tested at multiple levels:

```text
Unit Testing
Module Testing
Integration Testing
System Testing
Scenario Testing
GUI Testing
Final Acceptance Testing
```

---

# 37. Critical Acceptance Tests

The following must pass before final demonstration:

```text
CPU Execution
System Calls
Process Creation
Context Switching
Paging
Page Fault
Interrupt Handling
Multiprogramming
Scheduling
Synchronization
IPC
I/O
Buffering
Spooling
AI Agent Creation
AI Agent Scheduling
Agent Failure
Checkpoint Recovery
Full Integration
```

---

# 38. Success Metrics

The project will be considered successful when:

### OS Simulation

* Required Stage I concepts are implemented.
* Required Stage II concepts are implemented.
* Required Stage III concepts are implemented.
* Multiple processes can execute.
* Context switching works.
* Paging and page faults work.
* Interrupts work.
* Scheduling works.
* Synchronization works.
* IPC works.
* I/O, buffering, and spooling work.

### AI Layer

* Two AI agents can be created.
* Agents are represented as OS-managed workloads.
* Agents participate in scheduling.
* Agents communicate through IPC.
* Resource quotas can be simulated.
* Tool permissions can be simulated.
* Checkpoints can be created.
* Agent failure can be simulated.
* Agent recovery works.

### GUI

* The current OS state is visible.
* OS events are visible.
* Live updates work.
* Demo scenarios work.
* GUI values correspond to actual backend state.

---

# 39. Non-Functional Requirements

## Reliability

The simulator should not crash during normal demonstration scenarios.

## Reproducibility

Predefined scenarios should produce predictable results.

## Explainability

AI scheduling decisions should be understandable.

## Maintainability

OS subsystems should remain modular.

## Usability

A professor should be able to understand the system state from the Dashboard.

## Performance

The simulator should support the project's intended demonstration workload without noticeable UI lag.

---

# 40. Technical Stack

## OS Engine

```text
C++
```

Purpose:

* CPU simulation
* Process management
* Scheduling
* Memory management
* Interrupts
* IPC
* Synchronization
* I/O
* AI Agent Manager

---

## Frontend

```text
React
TypeScript
```

Purpose:

* Dashboard
* Visualization
* Controls
* Monitoring

---

## Styling

```text
Tailwind CSS
```

Purpose:

* Dark UI
* Responsive layout
* Components
* Styling

---

## Charts

```text
Recharts / Chart.js
```

Purpose:

* CPU utilization
* Scheduling comparison
* Performance metrics
* Timeline visualization

---

## Icons

```text
Lucide React
```

---

## Communication

```text
REST / HTTP
WebSocket
```

---

## Data Format

```text
JSON
```

---

## Build Tool

```text
Vite
```

---

## Version Control

```text
Git
GitHub
```

---


# 42. Major User Stories

## Student

> As a student, I want to run a predefined OS scenario so that I can observe how OS concepts work.

## Faculty

> As a faculty member, I want to see CPU, memory, processes, scheduling, interrupts, IPC, and I/O in one interface so that I can evaluate the implementation.

## Student

> As a user, I want to switch scheduling algorithms so that I can compare their behavior.

## User

> As a user, I want to see a page fault occur so that I can understand virtual memory behavior.

## User

> As a user, I want to observe IPC between AI agents so that I can see how agents communicate through the OS.

## User

> As a user, I want to simulate an agent failure and recover it from a checkpoint so that I can demonstrate fault recovery.

---

# 43. Key Product Principles

## Principle 1 — OS First

The project must remain an Operating System simulator.

AI functionality must not replace the core OS functionality.

---

## Principle 2 — AI as Workload

AI agents are simulated workloads managed by the OS.

```text
AI Agent
   ↓
Process
   ↓
Scheduler
   ↓
CPU
```

---

## Principle 3 — No Fake GUI

Every important GUI value must originate from the backend simulator.

For example:

```text
GUI:
Page Faults = 8
```

must mean:

```text
Backend:
8 actual page-fault events
```

---

## Principle 4 — Explainable AI

The AI Scheduler should explain why it selected a workload.

---

## Principle 5 — Integrated Simulation

OS concepts should interact.

The system should demonstrate:

```text
CPU
 ↓
Process
 ↓
Memory
 ↓
Interrupt
 ↓
Scheduler
 ↓
IPC / I/O
```

rather than isolated modules.

---

# 44. Major Risks

## Risk 1 — Scope Expansion

Adding real AI models or real autonomous agents could make the project too large.

### Mitigation

Use simulated AI agents.

---

## Risk 2 — GUI Before Backend

Building the complete dashboard before the simulator can produce real data could result in fake functionality.

### Mitigation

Develop backend modules first and connect GUI incrementally.

---

## Risk 3 — Integration Complexity

Individual modules may work but fail when connected.

### Mitigation

Perform integration testing continuously.

---

## Risk 4 — Two-Month Time Constraint

The full system contains many components.

### Mitigation

Prioritize:

```text
Core OS
   ↓
Required Stage I
   ↓
Required Stage II
   ↓
Required Stage III
   ↓
AI Layer
   ↓
GUI Refinement
```

---

# 45. MVP Definition

The Minimum Viable Product must include:

```text
CPU Simulation
Process Management
Paging
Page Faults
Interrupts
Multiprogramming
Scheduling
Synchronization
IPC
I/O
Buffering
Spooling
AI Agents
AI Adaptive Scheduler
Basic GUI
```

The following are secondary enhancements:

```text
Advanced Agent Permissions
Advanced Resource Quotas
Detailed Checkpoint Metadata
Advanced Analytics
Advanced Animations
```

The team must not sacrifice required OS functionality to implement optional AI features.

---

# 46. Definition of Done

A feature is considered complete when:

1. The C++ implementation exists.
2. The feature has a defined state model.
3. The feature has unit/module tests.
4. The feature integrates with relevant OS components.
5. The feature generates appropriate events.
6. The GUI can display the relevant state where applicable.
7. A demonstration scenario exists where appropriate.
8. The feature does not break existing functionality.

---

# 47. Final Product Flow

The complete AIOS product flow is:

```text
                     USER
                       │
                       ▼
                  REACT GUI
                       │
                REST / WebSocket
                       │
                       ▼
                 C++ OS ENGINE
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
      CPU          PROCESS         MEMORY
        │          MANAGER         MANAGER
        │              │              │
        │              ▼              ▼
        │          SCHEDULER      PAGE TABLE
        │              │              │
        └──────────────┼──────────────┘
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
    INTERRUPTS        IPC            I/O
        │              │              │
        └──────────────┼──────────────┘
                       │
                       ▼
                AI AGENT MANAGER
                       │
              ┌────────┴────────┐
              ▼                 ▼
        Research Agent     Coding Agent
              │                 │
              └────────┬────────┘
                       │
                       ▼
                AI ADAPTIVE
                  SCHEDULER
```

---

# 48. Final Product Statement

AIOS is a multiprogramming Operating System simulator that integrates fundamental OS mechanisms with simulated AI-agent workloads.

The system demonstrates:

```text
CPU
+
Processes
+
Memory
+
Paging
+
Virtual Memory
+
Interrupts
+
Scheduling
+
Synchronization
+
IPC
+
I/O
+
Buffering
+
Spooling
+
AI Agents
+
AI Adaptive Scheduling
+
Failure Recovery
```

through an interactive graphical interface.

The central design principle is:

> **AI agents operate inside the simulated operating system and are managed using operating-system mechanisms rather than existing outside them.**

The final product should demonstrate that AIOS is a cohesive Operating System simulation rather than a collection of independent feature demonstrations.

---

# 49. Project Success Definition

AIOS is successful if a faculty evaluator can observe one integrated simulation and trace:

```text
Process Creation
      ↓
Memory Allocation
      ↓
CPU Execution
      ↓
Scheduling
      ↓
Context Switching
      ↓
Page Fault
      ↓
Interrupt Handling
      ↓
I/O
      ↓
IPC
      ↓
Synchronization
      ↓
AI Agent Scheduling
      ↓
Agent Failure
      ↓
Checkpoint Recovery
      ↓
Continued Execution
```

while the GUI accurately reflects the internal state of the simulator.

**End of PRD**

````

## 🧩 Where `PRD.md` fits

Your repository should now look like:

```text
AIOS/
│
├── README.md                  ← entry point / quick start
├── LICENSE
├── AGENTS.md                  ← development-agent workflow and project rules
├── TEAMMATES.md               ← team and collaboration tracker
├── STATUS.md                  ← living project status
├── CHANGELOG.md
├── CONTRIBUTING.md
│
├── docs/                      ← product and subsystem specifications
│   ├── PRD.md                 ← product-level specification
│   ├── ROADMAP.md
│   ├── 01_Project_Overview.md
│   ├── 02_Requirements.md
│   ├── 03_System_Architecture.md
│   ├── 04_CPU_Machine_Simulation.md
│   ├── 05_System_Calls_Interrupts.md
│   ├── 06_Process_Management.md
│   ├── 07_Paging_Virtual_Memory.md
│   ├── 08_Scheduling.md
│   ├── 09_Synchronization.md
│   ├── 10_IPC.md
│   ├── 11_IO_Buffering_Spooling.md
│   ├── 12_AI_Agent_Layer.md
│   ├── 13_GUI_Architecture.md
│   ├── 14_Demo_Scenarios.md
│   └── 15_Testing.md
│
├── backend/                   ← C++ simulation engine (source of truth)
├── frontend/                  ← React dashboard
└── tests/                     ← unit and integration tests
````

**PRD = what we are building and why.**
**`docs/` = how each part is supposed to work.**

