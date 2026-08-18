# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 13. GUI Architecture

## 1. Purpose

This document defines the architecture and design of the graphical user interface for AIOS.

The GUI provides a visual representation of the internal state of the simulated operating system.

The GUI shall allow users to:

* Start the simulation.
* Pause the simulation.
* Stop the simulation.
* Reset the simulation.
* Monitor processes.
* Monitor AI agents.
* Observe CPU state.
* Observe memory and paging.
* Observe page faults.
* Observe interrupts.
* Observe scheduling.
* Observe IPC.
* Observe synchronization.
* Observe I/O devices.
* Observe event logs.
* Trigger selected demonstration scenarios.

The GUI is a monitoring and control interface.

The actual OS simulation is executed by the C++ OS Engine.

---

# 2. GUI Design Reference

The intended design follows a dark, professional operating-system dashboard.

The dashboard should provide a visual overview similar to:

```text
┌───────────────────────────────────────────────────────────────┐
│ AIOS SIMULATOR       START  PAUSE  STOP  RESET    Scheduler  │
├────────────┬──────────────────────────────────────────────────┤
│            │ CPU     Memory    Processes    Agents           │
│ Dashboard  │──────────────────────────────────────────────────│
│ Processes  │ Process List          AI Agents                 │
│ AI Agents  │                                                  │
│ CPU        │                                                  │
│ Memory     │                                                  │
│ I/O        │                                                  │
│ IPC        │                                                  │
│ Scheduler  │                                                  │
│ Interrupts │                                                  │
│ Logs       │                                                  │
└────────────┴──────────────────────────────────────────────────┘
```

The uploaded design reference shows the same overall approach:

```text
Sidebar
   +
Top Control Bar
   +
System Statistics
   +
OS Monitoring Panels
   +
Event / Communication Panels
```

---

# 3. GUI Technology

The frontend shall use:

```text
React
TypeScript
Tailwind CSS
Recharts / Chart.js
Lucide React
```

Communication:

```text
React Frontend
       │
       │ WebSocket
       ▼
C++ OS Engine
```

REST/HTTP may be used for:

```text
Configuration
Commands
Initial State
Simulation Setup
```

WebSocket shall be used for:

```text
Live Simulation Events
CPU Updates
Process State Changes
Memory Changes
Interrupts
IPC Messages
I/O Events
Scheduler Decisions
```

---

# 4. Frontend Architecture

The frontend should be divided into:

```text
Frontend
│
├── Layout
│
├── Components
│
├── Pages
│
├── Charts
│
├── State Management
│
├── API Layer
│
├── WebSocket Layer
│
└── Types
```

---

# 5. Recommended Frontend Structure

Conceptually:

```text
frontend/
│
├── src/
│   │
│   ├── components/
│   │   ├── Header/
│   │   ├── Sidebar/
│   │   ├── StatCard/
│   │   ├── ProcessTable/
│   │   ├── AgentTable/
│   │   ├── CPUPanel/
│   │   ├── MemoryGrid/
│   │   ├── PageTable/
│   │   ├── ReadyQueue/
│   │   ├── EventLog/
│   │   ├── IPCQueue/
│   │   ├── IODevices/
│   │   └── SchedulerPanel/
│   │
│   ├── pages/
│   │   ├── Dashboard/
│   │   ├── Processes/
│   │   ├── Agents/
│   │   ├── CPU/
│   │   ├── Memory/
│   │   ├── IO/
│   │   ├── IPC/
│   │   ├── Scheduler/
│   │   ├── Interrupts/
│   │   └── Logs/
│   │
│   ├── services/
│   │   ├── api.ts
│   │   └── websocket.ts
│   │
│   ├── types/
│   │   └── os.ts
│   │
│   └── App.tsx
│
└── package.json
```

The exact structure may be modified during implementation.

---

# 6. Overall Layout

The application shall use three major regions.

```text
┌──────────────────────────────────────────────┐
│                 TOP BAR                      │
├──────────────┬───────────────────────────────┤
│              │                               │
│   SIDEBAR    │         MAIN CONTENT          │
│              │                               │
│              │                               │
│              │                               │
└──────────────┴───────────────────────────────┘
```

---

# 7. Top Navigation Bar

The top bar contains global simulation controls.

Recommended controls:

```text
START
PAUSE
STOP
RESET
```

and:

```text
Scheduler
Clock Speed
Simulation Time
```

Example:

```text
┌────────────────────────────────────────────────────────────┐
│ AIOS SIMULATOR   ▶ Start  ❚❚ Pause  ■ Stop  ↻ Reset       │
│                                    Scheduler: AI Adaptive  │
│                                    Speed: 1.0x             │
└────────────────────────────────────────────────────────────┘
```

---

# 8. Start Button

The Start button starts the simulation.

Flow:

```text
User
 │
 ▼
START
 │
 ▼
Frontend Command
 │
 ▼
C++ OS Engine
 │
 ▼
Simulation Running
```

The UI should then begin receiving live events.

---

# 9. Pause Button

Pause temporarily stops simulation advancement.

It should preserve:

```text
CPU State
Process States
Memory State
Ready Queue
IPC Queues
I/O State
Simulation Time
```

The user can resume using Start/Resume.

---

# 10. Stop Button

Stop terminates the current simulation run.

The current simulation state may remain visible until Reset.

Example:

```text
RUNNING
   │
   ▼
STOP
   │
   ▼
STOPPED
```

---

# 11. Reset Button

Reset returns the simulator to its initial state.

```text
Reset
  │
  ▼
Clear Processes
Clear Memory
Clear Queues
Clear Interrupts
Clear IPC
Clear I/O
Reset CPU
Reset Metrics
  │
  ▼
Initial State
```

---

# 12. Clock Speed

The GUI shall provide a simulation-speed control.

Example:

```text
0.5x ─── 1x ─── 2x ─── 5x
```

This controls how quickly simulation cycles are visually presented.

It does not necessarily represent actual CPU clock frequency.

The distinction should be clear during implementation.

---

# 13. Scheduler Selector

The GUI should allow the user to select the scheduling algorithm.

Example:

```text
Scheduler:
┌─────────────────────┐
│ AI Adaptive       ▼ │
└─────────────────────┘
```

Options:

```text
FCFS
Round Robin
Priority
AI Adaptive
```

The selected algorithm is passed to the C++ Scheduler.

---

# 14. Sidebar

The sidebar provides navigation between OS subsystems.

Recommended pages:

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

This matches the structure shown in the design reference while keeping each subsystem connected to the project documentation.

---

# 15. Dashboard

The Dashboard is the main demonstration page.

It should provide an overview of the entire OS.

Recommended layout:

```text
Dashboard
│
├── System Statistics
│
├── Process List
├── AI Agents
├── CPU
├── Memory
├── Page Table
├── Ready Queue
├── Event Log
├── IPC Messages
└── I/O Devices
```

The professor should be able to understand the current system state from this page without opening every subsystem.

---

# 16. System Statistics

The top section should contain summary cards.

Recommended cards:

```text
Total Processes
AI Agents
CPU Usage
Memory Usage
Page Faults
Interrupts
```

Example:

```text
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│ Processes    │ │ AI Agents    │ │ CPU Usage    │
│      5       │ │      2       │ │      42%     │
└──────────────┘ └──────────────┘ └──────────────┘

┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│ Memory       │ │ Page Faults  │ │ Interrupts   │
│      68%     │ │      8       │ │      15      │
└──────────────┘ └──────────────┘ └──────────────┘
```

These values must come from the OS Engine.

They should never be hardcoded.

---

# 17. Process List

The Process List displays the current processes.

Recommended columns:

```text
PID
Type
Name
State
Priority
Memory
CPU Time
```

Example:

```text
┌────┬────────┬────────────┬─────────┬────────┬────────┐
│PID │ Type   │ Name       │ State   │Priority│ Memory │
├────┼────────┼────────────┼─────────┼────────┼────────┤
│P1  │Process │ TextEditor │Running  │2       │12 MB   │
│P2  │Process │ Compiler   │Ready    │1       │18 MB   │
│P3  │Process │ Player     │Waiting  │3       │10 MB   │
│A1  │Agent   │ Research   │Running  │1       │24 MB   │
│A2  │Agent   │ Coding     │Ready    │2       │20 MB   │
└────┴────────┴────────────┴─────────┴────────┴────────┘
```

---

# 18. Process State Visualization

Different process states should be visually distinguishable.

States:

```text
NEW
READY
RUNNING
WAITING
TERMINATED
FAILED
```

The exact colors should be defined by the frontend design system.

The important requirement is that the state is immediately recognizable.

---

# 19. AI Agent Panel

The AI Agent panel displays information specific to AI workloads.

Recommended columns:

```text
Agent ID
Name
PID
State
Task
Tokens
Permissions
```

Example:

```text
┌─────┬──────────────┬─────────┬─────────┬────────────┐
│ ID  │ Name         │ State   │ Task    │ Tokens     │
├─────┼──────────────┼─────────┼─────────┼────────────┤
│ A1  │ Research     │Running  │Research │1200/2000   │
│ A2  │ Coding       │Ready    │Coding   │800/1500    │
└─────┴──────────────┴─────────┴─────────┴────────────┘
```

---

# 20. CPU Page

The CPU page displays the state of the simulated CPU.

The CPU contains:

```text
PC
IR
ACC
MAR
MBR
FLAGS
```

Additional simulated registers may be displayed if implemented.

Example:

```text
┌──────────────────────────────┐
│ CPU                          │
├──────────┬───────────────────┤
│ PC       │ 1048              │
│ IR       │ LOAD R1, [2050]   │
│ ACC      │ 25                │
│ MAR      │ 2050              │
│ MBR      │ 15                │
│ FLAGS    │ Z=0 N=0 C=0       │
└──────────┴───────────────────┘
```

---

# 21. CPU Utilization Chart

The GUI should display CPU utilization over simulation time.

Example:

```text
CPU Utilization

100% ┤
 75% ┤        ╭──╮
 50% ┤   ╭────╯  ╰──╮
 25% ┤───╯          ╰──
  0% ┼────────────────────
       Time →
```

The chart should be generated from actual simulation data.

---

# 22. Memory Page

The Memory page visualizes:

```text
RAM
Pages
Frames
Page Table
Swap
```

Recommended layout:

```text
Memory
│
├── Memory Overview
├── Physical Frames
├── Page Table
├── Swap
└── Page Fault History
```

---

# 23. Memory Visualization

A grid can represent physical frames.

Example:

```text
Physical Memory

┌────┬────┬────┬────┬────┬────┐
│ P1 │ P1 │ A1 │ A1 │ P2 │Free│
├────┼────┼────┼────┼────┼────┤
│ P3 │ A2 │ A2 │Free│ P1 │ P2 │
└────┴────┴────┴────┴────┴────┘
```

Each frame should display its current owner.

---

# 24. Page Table Panel

The Page Table should show virtual-to-physical mappings.

Example:

```text
┌─────────┬────────┬───────┬──────────┬────────────┐
│ Page    │ Frame  │ Valid │ Modified │ Referenced │
├─────────┼────────┼───────┼──────────┼────────────┤
│ 0       │ 5      │ 1     │ 0        │ 1          │
│ 1       │ 12     │ 1     │ 1        │ 1          │
│ 2       │ -      │ 0     │ 0        │ 0          │
│ 3       │ 8      │ 1     │ 1        │ 1          │
└─────────┴────────┴───────┴──────────┴────────────┘
```

Invalid pages should be clearly indicated.

---

# 25. Page Fault Visualization

When a page fault occurs, the GUI should show:

```text
Page 2
   │
   ▼
Not Present
   │
   ▼
PAGE FAULT
   │
   ▼
Load Page
   │
   ▼
Frame Assigned
```

The Event Log should also record the event.

---

# 26. Ready Queue

The Scheduler page and Dashboard should display the Ready Queue.

Example:

```text
READY QUEUE

┌────┐     ┌────┐     ┌────┐     ┌────┐
│ P2 │ ──► │ P3 │ ──► │ A2 │ ──► │ P1 │
└────┘     └────┘     └────┘     └────┘
```

The queue should update whenever a process:

```text
Enters READY
Leaves READY
Is preempted
Completes
Blocks
```

---

# 27. Scheduler Panel

The Scheduler page should display:

```text
Current Algorithm
Current Process
Ready Queue
Time Quantum
Scheduling Score
Scheduling History
Metrics
```

For AI Adaptive:

```text
Algorithm:
AI Adaptive

Selected:
Research Agent

Score:
82.5

Reason:
High waiting time
High urgency
Moderate priority
```

---

# 28. Scheduling Comparison

The GUI should provide a comparison section.

Example:

```text
┌──────────────┬────────┬────────┬─────────┬──────────┐
│ Algorithm    │ Avg WT │ Avg RT │ CPU Util│ Switches │
├──────────────┼────────┼────────┼─────────┼──────────┤
│ FCFS         │   ...  │   ...  │   ...   │   ...    │
│ Round Robin  │   ...  │   ...  │   ...   │   ...    │
│ Priority     │   ...  │   ...  │   ...   │   ...    │
│ AI Adaptive  │   ...  │   ...  │   ...   │   ...    │
└──────────────┴────────┴────────┴─────────┴──────────┘
```

---

# 29. Gantt Chart

The Scheduler page should provide a Gantt-style timeline.

Example:

```text
CPU Timeline

0      5      10     15     20
│──────│──────│──────│──────│

P1 █████
P2      ███
A1          █████
P3                ███
A2                   ████
```

The chart must be generated from actual scheduling history.

---

# 30. Interrupt Page

The Interrupt page displays generated interrupts.

Example:

```text
┌────────┬───────────────────────┬─────────┐
│ Time   │ Interrupt             │ Status  │
├────────┼───────────────────────┼─────────┤
│ 11:24  │ Timer Interrupt       │Handled  │
│ 11:25  │ I/O Interrupt         │Handled  │
│ 11:26  │ Page Fault            │Handled  │
│ 11:27  │ System Call           │Handled  │
│ 11:28  │ IPC Event             │Handled  │
└────────┴───────────────────────┴─────────┘
```

---

# 31. Interrupt Statistics

The page may display:

```text
Total Interrupts
Handled
Pending
Page Faults
Timer Interrupts
I/O Interrupts
System Call Interrupts
```

---

# 32. IPC Page

The IPC page displays communication between processes and agents.

Recommended sections:

```text
Message Queue
Communication Graph
Message Details
IPC Statistics
```

---

# 33. IPC Message Queue

Example:

```text
┌─────────┬─────────┬────────────────────┐
│ Time    │ Route   │ Message            │
├─────────┼─────────┼────────────────────┤
│ 11:24   │ A1 → A2 │ Search Results     │
│ 11:24   │ A2 → A1 │ ACK                │
│ 11:25   │ A1 → A2 │ Additional Info    │
└─────────┴─────────┴────────────────────┘
```

---

# 34. IPC Communication Graph

The GUI may display:

```text
Research Agent
      │
      │ RESEARCH_RESULT
      ▼
Coding Agent
      │
      │ TASK_ACK
      ▼
Research Agent
```

This is especially useful during the viva because the professor can immediately see that IPC is actually being simulated.

---

# 35. Synchronization Page

The Synchronization page should display:

```text
Mutexes
Semaphores
Waiting Processes
Critical Sections
Deadlock Status
```

Example:

```text
┌────────┬──────────┬─────────┬────────────┐
│ Mutex  │ State    │ Owner   │ Waiting    │
├────────┼──────────┼─────────┼────────────┤
│ M1     │ LOCKED   │ P2      │ P3, A1     │
│ M2     │ FREE     │ -       │ -          │
└────────┴──────────┴─────────┴────────────┘
```

---

# 36. I/O Devices Page

The I/O page should display simulated devices.

Recommended devices:

```text
Disk
Keyboard
Network
Printer
```

Example:

```text
┌──────────┬───────────┬────────────────┐
│ Device   │ State     │ Queue          │
├──────────┼───────────┼────────────────┤
│ Disk     │ BUSY      │ P2 → A1        │
│ Keyboard │ AVAILABLE │ -              │
│ Network  │ BUSY      │ A2             │
│ Printer  │ AVAILABLE │ P3             │
└──────────┴───────────┴────────────────┘
```

---

# 37. I/O Queue

The selected device should display its request queue.

Example:

```text
Disk Queue

P2
 ↓
A1
 ↓
P3
```

The queue updates dynamically as I/O requests are processed.

---

# 38. Buffer Visualization

The I/O page may display buffer usage.

Example:

```text
Output Buffer

[██████████████░░░░░░]

70%

Used: 14
Capacity: 20
```

---

# 39. Spooling Visualization

The Printer page can show:

```text
Printer Spool Queue

Job 101 → P1
Job 102 → A1
Job 103 → P3
Job 104 → A2

             ↓

          PRINTER
           BUSY
```

This makes the spooling requirement visible.

---

# 40. Logs Page

The Logs page contains a detailed chronological event stream.

Example:

```text
11:24:10 Process P1 created.
11:24:11 Process P2 created.
11:24:12 AI Agent A1 created.
11:24:13 AI Agent A2 created.
11:24:18 Scheduler started.
11:24:20 Context switch P1 → A1.
11:24:25 Page fault occurred.
11:24:26 Page loaded from disk.
11:24:30 IPC message sent A1 → A2.
11:24:32 A2 received message.
11:24:33 A2 crashed.
11:24:34 Recovering A2 from checkpoint.
11:24:35 A2 recovered successfully.
```

---

# 41. Event Log Categories

Events should be categorized.

Possible categories:

```text
PROCESS
CPU
MEMORY
SCHEDULER
INTERRUPT
IPC
I/O
SYNCHRONIZATION
AI_AGENT
ERROR
```

This allows filtering.

---

# 42. Log Filtering

The GUI should allow filtering.

Example:

```text
[All] [Process] [CPU] [Memory] [IPC] [I/O] [AI]
```

Selecting:

```text
IPC
```

shows only IPC events.

---

# 43. Settings Page

The Settings page contains simulation configuration.

Possible settings:

```text
Memory Size
Page Size
Number of Frames
Time Quantum
Scheduling Algorithm
Clock Speed
Simulation Seed
I/O Device Configuration
AI Scheduler Weights
```

---

# 44. AI Scheduler Configuration

The AI Scheduler weights can be configurable.

Example:

```text
Priority Weight
[██████──────] 0.30

Waiting Weight
[██████──────] 0.30

Urgency Weight
[█████───────] 0.25

Resource Weight
[███─────────] 0.15
```

The actual values should be sent to the C++ Scheduler.

---

# 45. Scenario Controls

The GUI should provide predefined demonstration scenarios.

Recommended scenarios:

```text
Basic Multiprogramming
Paging / Page Fault
Interrupt Handling
IPC Communication
Synchronization
I/O + Spooling
AI Agent Failure
Scheduling Comparison
Full System Demo
```

This is better than requiring the professor to manually configure every subsystem.

---

# 46. Basic Multiprogramming Scenario

The GUI command:

```text
START SCENARIO
Basic Multiprogramming
```

creates:

```text
P1
P2
P3
A1
A2
```

Then:

```text
Load Memory
   ↓
Start Scheduler
   ↓
Context Switch
   ↓
CPU Execution
```

---

# 47. Page Fault Scenario

The GUI should provide:

```text
Generate Page Fault
```

or a predefined scenario.

Flow:

```text
Memory Access
     ↓
Page Not Present
     ↓
Page Fault
     ↓
Interrupt
     ↓
Page Load
     ↓
Resume Process
```

The GUI should update:

```text
Page Table
Memory Grid
Interrupt Log
Event Log
Process State
```

---

# 48. IPC Scenario

The GUI should allow:

```text
Run IPC Demo
```

Flow:

```text
Research Agent
      ↓
TASK_REQUEST
      ↓
IPC Queue
      ↓
Coding Agent
      ↓
TASK_RESULT
      ↓
Research Agent
```

---

# 49. Failure Recovery Scenario

The GUI should allow:

```text
Simulate Agent Crash
```

Flow:

```text
A2 RUNNING
    ↓
CRASH
    ↓
FAILED
    ↓
CHECKPOINT
    ↓
RECOVERY
    ↓
READY
    ↓
RUNNING
```

The Event Log should clearly display every step.

---

# 50. Full Demonstration Scenario

A complete demo can be triggered with:

```text
FULL OS DEMO
```

The scenario should execute:

```text
1. Create processes
2. Create AI agents
3. Load memory
4. Start scheduler
5. Context switch
6. Generate page fault
7. Handle interrupt
8. Perform I/O
9. Demonstrate IPC
10. Demonstrate synchronization
11. Simulate agent crash
12. Recover checkpoint
13. Continue execution
14. Display scheduling comparison
```

This is the most important scenario for the final project demonstration.

---

# 51. GUI-to-OS Communication

The frontend should never directly modify internal OS structures.

Instead:

```text
GUI
 │
 ▼
API / WebSocket Layer
 │
 ▼
C++ OS Engine
 │
 ├── Process Manager
 ├── Scheduler
 ├── Memory Manager
 ├── Interrupt Manager
 ├── IPC Manager
 ├── Synchronization Manager
 ├── I/O Manager
 └── AI Agent Manager
```

This prevents the frontend from becoming part of the OS logic.

---

# 52. Command Communication

For commands:

```text
GUI
 │
 │ POST /command
 ▼
C++ API
 │
 ▼
OS Engine
```

Example command:

```json
{
    "command": "START_SIMULATION"
}
```

Other commands:

```text
PAUSE_SIMULATION
STOP_SIMULATION
RESET_SIMULATION
CHANGE_SCHEDULER
RUN_SCENARIO
GENERATE_PAGE_FAULT
SIMULATE_AGENT_FAILURE
```

---

# 53. Live State Updates

The OS Engine should send live state updates through WebSocket.

Example:

```text
C++ OS Engine
      │
      │ WebSocket Event
      ▼
Frontend
```

Example:

```json
{
    "type": "PROCESS_STATE_CHANGED",
    "pid": 4,
    "old_state": "READY",
    "new_state": "RUNNING"
}
```

The frontend updates the corresponding panel.

---

# 54. Event-Driven GUI

The GUI should be event-driven.

Instead of repeatedly asking:

```text
"Is anything changed?"
```

the backend sends an event when something changes.

Example:

```text
Page Fault
    │
    ▼
OS Event
    │
    ▼
WebSocket
    │
    ▼
Frontend
    │
    ├── Update Page Table
    ├── Update Memory
    ├── Update Interrupts
    └── Update Event Log
```

---

# 55. Shared State Model

The frontend should maintain a representation of the current OS state.

Conceptually:

```text
OSState
│
├── simulation
├── cpu
├── processes
├── agents
├── memory
├── scheduler
├── interrupts
├── ipc
├── synchronization
├── io
└── logs
```

---

# 56. TypeScript State Types

The frontend should define TypeScript interfaces for OS data.

Example:

```text
ProcessState
├── pid
├── name
├── type
├── state
├── priority
├── memory
└── cpuTime
```

Agent:

```text
AgentState
├── agentId
├── name
├── pid
├── state
├── task
├── tokenUsage
└── permissions
```

CPU:

```text
CPUState
├── pc
├── ir
├── acc
├── mar
├── mbr
└── flags
```

---

# 57. GUI Update Frequency

The simulator should avoid sending excessive updates.

Possible approach:

```text
CPU/Register Updates
→ frequent

Process State Updates
→ event-based

Memory Changes
→ event-based

IPC
→ event-based

Logs
→ event-based
```

This reduces unnecessary frontend processing.

---

# 58. Dashboard Information Hierarchy

The Dashboard should prioritize the most important information.

### Level 1 — Overall System

```text
CPU
Memory
Processes
Agents
Interrupts
Page Faults
```

### Level 2 — Current Activity

```text
Running Process
Ready Queue
Current Scheduler
Current I/O
Recent IPC
```

### Level 3 — Detailed Information

```text
Page Table
CPU Registers
Event Logs
Synchronization State
```

---

# 59. Dashboard Layout

Recommended final layout:

```text
┌───────────────────────────────────────────────────────────────┐
│                        TOP BAR                                │
├────────────┬──────────────────────────────────────────────────┤
│            │  STAT CARDS                                      │
│            ├──────────────────────────────────────────────────┤
│            │  PROCESS LIST          AI AGENTS                 │
│ SIDEBAR    ├───────────────────────┬──────────────────────────┤
│            │  MEMORY / PAGING      │ CPU                      │
│            │                       │                          │
│            ├───────────────────────┼──────────────────────────┤
│            │  READY QUEUE           │ INTERRUPTS               │
│            ├───────────────────────┼──────────────────────────┤
│            │  EVENT LOG             │ IPC MESSAGE QUEUE        │
│            ├───────────────────────┴──────────────────────────┤
│            │  I/O DEVICES                                     │
└────────────┴──────────────────────────────────────────────────┘
```

This is very close to the provided design reference and is a good target for the implementation.

---

# 60. Responsive Behaviour

The GUI should support different screen sizes.

For the final demonstration, desktop/laptop layout is the primary target.

The minimum supported target should be:

```text
1366 × 768
```

The layout should avoid requiring horizontal scrolling for the primary dashboard.

---

# 61. Visual Design

The UI should use:

```text
Dark Background
Dark Panels
Subtle Borders
Clear Typography
Status Indicators
Minimal Animations
Charts
Tables
Icons
```

The visual design should communicate:

```text
Operating System
+
System Monitoring
+
AI
```

without becoming visually overloaded.

---

# 62. Status Indicators

The GUI should clearly indicate states such as:

```text
Running
Ready
Waiting
Terminated
Failed
Busy
Available
Locked
Free
Page Fault
Handled
Pending
```

Status indicators should be consistent across all pages.

---

# 63. Live Animation

Animations may be used for:

```text
Context Switch
IPC Message
Page Fault
Interrupt
I/O Request
Agent Failure
Agent Recovery
```

Example:

```text
A1 ───────────────► A2
        IPC
```

The animation should be short and informative.

It should not replace actual event data.

---

# 64. GUI Error Handling

If the backend returns an error:

```text
Backend
   │
   ▼
Error Event
   │
   ▼
Frontend
```

The GUI should display a clear message.

Example:

```text
⚠ IPC ERROR

Receiver PID 8 does not exist.
```

Errors should also appear in the Event Log.

---

# 65. Backend Disconnection

If the WebSocket connection is lost:

```text
C++ Engine
     X
Frontend
```

The GUI should display:

```text
OS Engine: Disconnected
```

The dashboard should not pretend that the simulation is still receiving live updates.

---

# 66. GUI and Project Requirements

The GUI must visibly demonstrate all major required OS concepts.

| Requirement            | GUI Representation            |
| ---------------------- | ----------------------------- |
| CPU Simulation         | CPU Panel                     |
| Supervisor Calls       | Interrupt/System Call Log     |
| Paging                 | Memory Grid + Page Table      |
| Error Handling         | Error Log                     |
| Interrupts             | Interrupt Panel               |
| Process Data Structure | Process Table                 |
| Multiprogramming       | Multiple Processes            |
| Virtual Memory         | Pages + Frames + Swap         |
| Scheduling             | Scheduler Panel + Gantt Chart |
| Synchronization        | Mutex/Semaphore Panel         |
| IPC                    | Message Queue                 |
| I/O                    | Device Panel                  |
| Buffering              | Buffer Visualization          |
| Spooling               | Spool Queue                   |
| AI Agents              | AI Agent Panel                |
| Failure Recovery       | Agent Status + Event Log      |

---

# 67. Important Design Principle

Every major visualization must correspond to an actual backend state.

For example:

```text
GUI says:
"Page Fault = 8"
```

Therefore the C++ simulator must actually have generated eight page-fault events.

Similarly:

```text
GUI says:
"AI Agent A2 recovered"
```

because the backend actually performed:

```text
FAILED
   ↓
CHECKPOINT RESTORE
   ↓
READY
```

The GUI shall not contain fake OS behaviour.

---

# 68. Dashboard Data Sources

Each panel should have a clear backend source.

```text
Process List
      ↓
Process Manager

AI Agents
      ↓
AI Agent Manager

CPU
      ↓
CPU Simulator

Memory
      ↓
Memory Manager

Ready Queue
      ↓
Scheduler

Interrupts
      ↓
Interrupt Manager

IPC
      ↓
IPC Manager

I/O
      ↓
I/O Manager

Synchronization
      ↓
Synchronization Manager

Logs
      ↓
OS Event Manager
```

---

# 69. GUI Architecture Summary

The final architecture is:

```text
                         FRONTEND
                            │
              ┌─────────────┴─────────────┐
              │                           │
          React UI                  State Manager
              │                           │
              └─────────────┬─────────────┘
                            │
                     API / WebSocket
                            │
                            ▼
                     C++ OS ENGINE
                            │
       ┌────────────────────┼────────────────────┐
       │                    │                    │
       ▼                    ▼                    ▼
 Process Manager        Scheduler           Memory Manager
       │                    │                    │
       ▼                    ▼                    ▼
       CPU             Interrupts               IPC
                            │                    │
                            ▼                    ▼
                         I/O              Synchronization
                            │
                            ▼
                    AI Agent Manager
```

---

# 70. Final Dashboard Objective

The dashboard should allow a professor to look at one screen and understand:

```text
How many processes exist?
        ↓
Which process is running?
        ↓
Which AI agents exist?
        ↓
Who owns the CPU?
        ↓
How is memory being used?
        ↓
Did a page fault occur?
        ↓
Did an interrupt occur?
        ↓
What is in the Ready Queue?
        ↓
Which scheduling algorithm is active?
        ↓
Are agents communicating?
        ↓
Are I/O devices busy?
        ↓
Did an agent fail?
        ↓
Was it recovered?
```

The GUI is therefore not merely a frontend.

It is the **visual proof of the operating-system simulation**.

---

# 71. Success Criteria

The GUI subsystem is complete when:

* The simulator can be started.
* The simulator can be paused.
* The simulator can be stopped.
* The simulator can be reset.
* The current scheduler is visible.
* Processes are displayed.
* AI agents are displayed.
* CPU registers are displayed.
* CPU utilization is displayed.
* Memory usage is displayed.
* Pages and frames are visualized.
* Page tables are displayed.
* Page faults are displayed.
* Interrupts are displayed.
* Ready Queue is displayed.
* Scheduling decisions are displayed.
* Scheduling comparison is displayed.
* IPC messages are displayed.
* Synchronization state is displayed.
* I/O devices are displayed.
* Buffers/spooling can be displayed.
* Event logs are displayed.
* Agent failure and recovery are visible.
* Backend state is reflected accurately in the UI.
* The frontend receives live updates from the C++ OS Engine.

---

# 72. Summary

The AIOS GUI is a visual control and monitoring layer over the simulated operating system.

The architecture is:

```text
                React + TypeScript
                       │
                 Tailwind CSS
                       │
              Charts / Components
                       │
                WebSocket / REST
                       │
                       ▼
                  C++ OS Engine
                       │
       ┌───────────────┼────────────────┐
       │               │                │
       ▼               ▼                ▼
    Process         Scheduler         Memory
    Manager                              │
       │               │                │
       ├───────────────┼────────────────┤
       │               │                │
       ▼               ▼                ▼
      IPC        Synchronization        I/O
       │
       ▼
 AI Agent Manager
```

The Dashboard acts as the central view, while individual pages provide detailed views of each subsystem.

The provided UI reference is an appropriate design direction because it visually connects the major components of AIOS:

```text
Processes
AI Agents
CPU
Memory
Paging
Ready Queue
Scheduler
Interrupts
IPC
I/O
Logs
```

The final implementation should preserve this structure while ensuring that every displayed value comes from the actual C++ simulation engine.
