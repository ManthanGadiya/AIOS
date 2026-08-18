# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 8. Process Scheduling

## 1. Purpose

This document defines the process scheduling subsystem of AIOS.

The Scheduler is responsible for selecting the next process from the Ready Queue and allocating the simulated CPU to it.

AIOS shall implement both classical scheduling algorithms and an AI Adaptive Scheduler.

The scheduling subsystem covers:

* Ready Queue management
* CPU allocation
* Context switching
* FCFS
* Round Robin
* Priority Scheduling
* AI Adaptive Scheduling
* Scheduling metrics
* Starvation
* Fairness
* Scheduling events
* Scheduling comparison

---

# 2. Scheduling Concept

In a multiprogramming system, multiple processes may be ready to execute while only one process can use the simulated CPU at a time.

The Scheduler decides:

> **Which ready process should execute next?**

Basic flow:

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

---

# 3. Scheduling Architecture

The Scheduler contains multiple scheduling policies.

```text
Scheduler
│
├── FCFS
├── Round Robin
├── Priority Scheduling
└── AI Adaptive Scheduling
```

Only one policy is active during a particular simulation run.

The user shall be able to select the scheduling algorithm through the dashboard.

---

# 4. Scheduler Responsibilities

The Scheduler is responsible for:

* Selecting the next process.
* Applying the selected scheduling algorithm.
* Managing scheduling decisions.
* Triggering context switches when required.
* Handling preemption where applicable.
* Maintaining scheduling statistics.
* Producing scheduling events.
* Supporting comparison between algorithms.

The Scheduler shall not:

* Execute CPU instructions.
* Manage page tables.
* Directly allocate memory.
* Directly perform I/O.
* Directly implement IPC.
* Directly modify AI-agent permissions.

Those responsibilities belong to other components.

---

# 5. Ready Queue

The Ready Queue contains processes that are ready to use the CPU.

Example:

```text
Ready Queue

┌────┬────┬────┬────┬────┐
│ P1 │ P2 │ P3 │ A1 │ A2 │
└────┴────┴────┴────┴────┘
```

The Scheduler selects one process according to the active policy.

A process leaves the Ready Queue when it becomes:

```text
RUNNING
```

It may return to the Ready Queue after:

```text
Preemption
Time Quantum Expiration
Wake-up
```

---

# 6. Scheduling Decision

A scheduling decision can be triggered by:

```text
Process Completion
Timer Interrupt
Time Quantum Expiration
Process Blocking
I/O Completion
Higher-Priority Process
Manual Simulation Event
```

General flow:

```text
Scheduling Event
      │
      ▼
Scheduler
      │
      ▼
Read Ready Queue
      │
      ▼
Apply Scheduling Policy
      │
      ▼
Select Process
      │
      ▼
Context Switch if Required
      │
      ▼
CPU
```

---

# 7. FCFS Scheduling

## 7.1 Concept

First Come First Serve schedules processes according to their arrival order.

The process that enters the Ready Queue first is selected first.

Example:

```text
Arrival Order:

P1 → P2 → P3

Execution:

P1 → P2 → P3
```

---

## 7.2 FCFS Behaviour

Suppose:

```text
P1 Arrival = 0
P2 Arrival = 1
P3 Arrival = 2
```

The execution order is:

```text
P1
 ↓
P2
 ↓
P3
```

Once a process starts running, it normally continues until completion or blocking.

---

## 7.3 FCFS Queue

FCFS uses a FIFO queue.

```text
Front
  ↓
┌────┬────┬────┐
│ P1 │ P2 │ P3 │
└────┴────┴────┘
                ↑
              Back
```

---

# 8. Round Robin Scheduling

## 8.1 Concept

Round Robin gives every ready process a fixed amount of CPU time called a **time quantum**.

Example:

```text
Time Quantum = 3 cycles
```

Execution:

```text
P1 → 3 cycles
P2 → 3 cycles
P3 → 3 cycles
P1 → 3 cycles
...
```

---

# 9. Round Robin Flow

```text
Ready Queue
    │
    ▼
   P1
    │
    │ Quantum expires
    ▼
   P2
    │
    │ Quantum expires
    ▼
   P3
    │
    │ Quantum expires
    ▼
   P1
```

When a process's quantum expires and it has not completed:

```text
RUNNING → READY
```

The process is placed at the back of the Ready Queue.

---

# 10. Time Quantum

The time quantum is configurable.

Example:

```text
TIME_QUANTUM = 4 cycles
```

A smaller quantum generally creates more context switches.

A larger quantum allows processes to execute for longer periods before switching.

The dashboard should display the selected quantum.

---

# 11. Priority Scheduling

## 11.1 Concept

Priority Scheduling selects the process with the highest scheduling priority.

Example:

```text
P1 → Priority 3
P2 → Priority 8
P3 → Priority 5
```

If larger values represent higher priority:

```text
P2 → selected first
```

---

# 12. Priority Queue

Conceptually:

```text
Highest Priority
      ↓
┌──────────────┐
│ P2 : 8       │
├──────────────┤
│ P3 : 5       │
├──────────────┤
│ P1 : 3       │
└──────────────┘
```

The exact priority convention shall be fixed in the implementation configuration and displayed clearly in the UI.

---

# 13. Preemptive vs Non-Preemptive Scheduling

AIOS shall distinguish between:

### Non-Preemptive

Once a process receives the CPU, it continues until it:

```text
Terminates
or
Blocks
```

### Preemptive

The OS can take the CPU away from a running process.

Examples:

```text
Timer Interrupt
Time Quantum Expiration
Higher Priority Process
```

Round Robin shall use preemption.

FCFS shall initially use non-preemptive behaviour.

Priority Scheduling may support the configured preemptive policy.

---

# 14. AI Adaptive Scheduling

The AI Adaptive Scheduler is the project's intelligent scheduling extension.

It does not use an external AI model.

Instead, it uses predefined heuristics to calculate a scheduling score.

The goal is:

> **Make scheduling decisions using multiple dynamic characteristics rather than relying on a single fixed scheduling rule.**

---

# 15. AI Scheduling Inputs

The AI Scheduler may consider:

```text
Base Priority
Waiting Time
CPU Usage
Memory Usage
Task Urgency
Resource Requirements
Page Fault Activity
```

The exact set can be reduced if required during implementation.

The important requirement is that the decision should be **explainable**.

---

# 16. AI Scheduling Score

The AI Scheduler may calculate a score conceptually as:

```text
Scheduling Score =
    Priority Factor
  + Waiting Factor
  + Urgency Factor
  - Resource Cost
```

For example:

```text
Score =
    Wp × Priority
  + Ww × WaitingTime
  + Wu × Urgency
  - Wr × ResourceUsage
```

where:

```text
Wp = Priority weight
Ww = Waiting-time weight
Wu = Urgency weight
Wr = Resource-usage weight
```

The weights shall be configurable.

The final formula should be documented in the implementation once experimentally tuned.

---

# 17. Why Use a Heuristic?

The project does not require a machine-learning model.

A heuristic is useful because:

* It is deterministic.
* It is easy to explain.
* It can be tested.
* Its decisions can be visualized.
* It does not require external APIs.
* It keeps the project focused on Operating Systems.

Example:

```text
Agent A
Priority = 5
Waiting Time = High
Urgency = High

Agent B
Priority = 8
Waiting Time = Low
Urgency = Low
```

The AI Scheduler may select Agent A if its waiting-time and urgency contributions outweigh Agent B's higher base priority.

---

# 18. Explainable AI Scheduling

Every AI scheduling decision should generate an explanation.

Example:

```text
AI Scheduler Decision

Selected:
Research Agent

Score:
82.5

Factors:
+ High waiting time
+ High task urgency
+ Moderate priority
- Moderate memory usage
```

This makes the AI component useful during the viva because the team can explain **why** a process was selected.

---

# 19. AI Scheduler and Normal Processes

The AI Scheduler does not need to schedule only AI agents.

Normal processes can also be evaluated.

Example:

```text
P1 → Normal Process
P2 → Normal Process
A1 → AI Agent
A2 → AI Agent
```

The scheduler can calculate a score for all ready workloads.

Therefore:

```text
AI Scheduler
     │
     ├── P1
     ├── P2
     ├── A1
     └── A2
```

---

# 20. Scheduling AI Agents

AI agents have additional scheduling information.

Example:

```text
Research Agent
├── Priority = 7
├── CPU Usage = 20%
├── Memory Usage = 30%
├── Urgency = 9
└── Waiting Time = 5 cycles
```

The AI Scheduler can use these properties when calculating its scheduling score.

---

# 21. Scheduling and Context Switching

The Scheduler does not directly manipulate CPU registers.

Instead:

```text
Scheduler
    │
    ▼
Selected Process
    │
    ▼
Process Manager
    │
    ├── Save Current Context
    │
    └── Load New Context
    │
    ▼
CPU
```

This maintains separation between scheduling and process management.

---

# 22. Context Switch Example

Suppose:

```text
P1 = RUNNING
Ready Queue = [P2, P3, A1]
```

A timer interrupt occurs.

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
Select P2
    │
    ▼
Save P1 Context
    │
    ▼
Load P2 Context
    │
    ▼
P2 Running
```

---

# 23. Scheduling and Process States

The Scheduler operates primarily on:

```text
READY
RUNNING
```

Example:

```text
READY
  │
  │ Scheduler selects
  ▼
RUNNING
  │
  ├── Completion ─────► TERMINATED
  │
  ├── I/O ────────────► WAITING
  │
  └── Preemption ─────► READY
```

---

# 24. Scheduling and Interrupts

Interrupts frequently trigger scheduling decisions.

### Timer Interrupt

```text
Timer
 ↓
Interrupt Manager
 ↓
Scheduler
 ↓
Context Switch
```

### I/O Completion

```text
I/O Complete
 ↓
Interrupt Manager
 ↓
Waiting Process → READY
 ↓
Scheduler
```

### Process Termination

```text
Process Terminated
 ↓
Scheduler
 ↓
Select Next Process
```

---

# 25. Scheduling Metrics

AIOS shall collect metrics to compare scheduling algorithms.

Required metrics:

```text
Waiting Time
Turnaround Time
Response Time
CPU Utilization
Throughput
Context Switch Count
```

---

# 26. Waiting Time

Waiting time is the time a process spends waiting in the Ready Queue.

Conceptually:

```text
Waiting Time =
Time spent READY
```

Example:

```text
P1 enters READY at cycle 2
P1 starts RUNNING at cycle 7

Waiting Time = 5 cycles
```

---

# 27. Turnaround Time

Turnaround time measures the total time from process arrival until completion.

```text
Turnaround Time =
Completion Time - Arrival Time
```

Example:

```text
Arrival = 0
Completion = 15

Turnaround Time = 15 cycles
```

---

# 28. Response Time

Response time measures how long a process waits before receiving CPU time for the first time.

```text
Response Time =
First CPU Start - Arrival Time
```

Example:

```text
Arrival = 2
First CPU Start = 6

Response Time = 4 cycles
```

---

# 29. CPU Utilization

CPU utilization measures how much of the simulation time the CPU is actively executing a process.

Conceptually:

```text
CPU Utilization =
Busy CPU Time / Total Simulation Time × 100
```

Example:

```text
CPU Busy = 80 cycles
Total = 100 cycles

CPU Utilization = 80%
```

---

# 30. Throughput

Throughput represents how many processes complete within a given simulation period.

```text
Throughput =
Completed Processes / Simulation Time
```

The dashboard may display throughput for each scheduling algorithm.

---

# 31. Context Switch Count

The simulator shall count context switches.

Example:

```text
FCFS
Context Switches = 2

Round Robin
Context Switches = 12
```

This allows the team to explain the trade-off between fairness and context-switch overhead.

---

# 32. Scheduling Comparison

AIOS shall allow the same workload to be executed using different algorithms.

Example workload:

```text
P1
P2
P3
Research Agent
Coding Agent
```

The simulator can execute:

```text
FCFS
   ↓
Collect Metrics

Round Robin
   ↓
Collect Metrics

Priority
   ↓
Collect Metrics

AI Adaptive
   ↓
Collect Metrics
```

The results can then be compared.

---

# 33. Scheduling Comparison Table

The dashboard should provide a table similar to:

```text
┌──────────────────┬─────────┬─────────┬─────────┬──────────┐
│ Algorithm        │ Avg WT  │ Avg RT  │ CPU Util│ Switches │
├──────────────────┼─────────┼─────────┼─────────┼──────────┤
│ FCFS             │   ...   │   ...   │   ...   │   ...    │
│ Round Robin      │   ...   │   ...   │   ...   │   ...    │
│ Priority         │   ...   │   ...   │   ...   │   ...    │
│ AI Adaptive      │   ...   │   ...   │   ...   │   ...    │
└──────────────────┴─────────┴─────────┴─────────┴──────────┘
```

The values shall be generated from actual simulation results rather than hardcoded.

---

# 34. Gantt Chart

AIOS should visualize process execution using a Gantt-style timeline.

Example:

```text
Time →

0    3    6    9    12   15

P1 |----|
P2      |----|
P3           |----|
A1                |----|
A2                     |----|
```

For Round Robin:

```text
Time →

0  2  4  6  8  10 12

P1 |--|      |--|
P2    |--|      |--|
P3       |--|      |--|
```

This makes scheduling behaviour easy to understand during the demo.

---

# 35. Starvation

Starvation occurs when a process waits for a very long time because other processes repeatedly receive CPU time.

Example:

```text
High Priority:
P1 → P2 → P1 → P2 → P1 → P2

Low Priority:
P3 → WAITING...
```

AIOS shall track waiting time so that potential starvation can be identified.

---

# 36. Aging

Aging can be used to reduce starvation in priority-based scheduling.

Conceptually:

```text
Long Waiting Time
       ↓
Increase Effective Priority
       ↓
Process becomes more likely
to receive CPU
```

If implemented, aging should affect the **effective scheduling score/priority**, not necessarily the original base priority stored in the PCB.

---

# 37. Fairness

The scheduler should provide reasonable CPU access to competing workloads.

Fairness can be improved through:

```text
Round Robin
Waiting-Time Factors
Aging
AI Scheduling Heuristics
```

Fairness should be evaluated using simulation statistics rather than assumed.

---

# 38. AI Scheduler Constraints

The AI Adaptive Scheduler shall follow these constraints:

1. It shall not bypass the Process Manager.
2. It shall not directly manipulate CPU registers.
3. It shall not directly allocate memory.
4. It shall use information provided by OS components.
5. Its decisions shall be explainable.
6. Its scoring mechanism shall be deterministic for the same simulation state.
7. It shall be possible to compare its results against classical algorithms.

---

# 39. Scheduler Configuration

The scheduler shall expose configurable parameters.

Example:

```text
Scheduling Algorithm:
AI Adaptive

Priority Weight:
0.30

Waiting Weight:
0.30

Urgency Weight:
0.25

Resource Weight:
0.15
```

For Round Robin:

```text
Scheduling Algorithm:
Round Robin

Time Quantum:
4 cycles
```

The configuration can be provided through JSON or the dashboard.

---

# 40. Scheduling Events

The Scheduler shall generate events such as:

```text
SCHEDULER_STARTED
PROCESS_SELECTED
PROCESS_PREEMPTED
TIME_QUANTUM_EXPIRED
CONTEXT_SWITCH
PROCESS_COMPLETED
AI_SCHEDULING_DECISION
```

Example:

```text
{
    "type": "AI_SCHEDULING_DECISION",
    "selected_pid": 4,
    "score": 82.5,
    "reason": [
        "high_waiting_time",
        "high_urgency"
    ]
}
```

---

# 41. Scheduler Interface

The Scheduler should expose functionality conceptually similar to:

```text
Scheduler
│
├── setAlgorithm()
├── schedule()
├── preempt()
├── getNextProcess()
├── setTimeQuantum()
├── calculateMetrics()
├── getStatistics()
└── getSchedulingHistory()
```

AI-specific logic may be separated internally:

```text
Scheduler
│
├── FCFS
├── RoundRobin
├── PriorityScheduler
└── AIScheduler
```

---

# 42. Example: FCFS Simulation

Suppose:

```text
P1 Burst = 5
P2 Burst = 3
P3 Burst = 4
```

Execution:

```text
0        5        8        12
|   P1   |   P2   |   P3    |
```

The Scheduler selects processes according to arrival order.

---

# 43. Example: Round Robin Simulation

Suppose:

```text
Time Quantum = 2

P1 = 5 cycles
P2 = 3 cycles
P3 = 4 cycles
```

Possible execution:

```text
0  2  4  6  8  10 12

P1 |--|
P2    |--|
P3       |--|
P1          |--|
P2             |-|
P3               |--|
P1                    |-|
```

The exact timeline shall be generated by the simulator.

---

# 44. Example: AI Adaptive Scheduling

Suppose the Ready Queue contains:

```text
P1
P2
Research Agent
Coding Agent
```

The scheduler calculates:

```text
P1              → Score 42
P2              → Score 55
Research Agent  → Score 78
Coding Agent    → Score 63
```

Therefore:

```text
Selected:
Research Agent
```

The simulator should display the factors responsible for the decision.

---

# 45. AI Scheduling and Resource Usage

The AI scheduler may consider resource consumption.

Example:

```text
Agent A
CPU Usage = High
Memory Usage = Low

Agent B
CPU Usage = Low
Memory Usage = Moderate
Waiting Time = High
```

The scheduler can balance these factors rather than simply selecting the process with the highest base priority.

The purpose is not to claim that the heuristic is universally optimal.

The purpose is to demonstrate an adaptive scheduling strategy.

---

# 46. Scheduling and Memory

The AI scheduler may receive information such as:

```text
Page Fault Count
Memory Usage
```

However, memory allocation and page replacement remain the responsibility of the Memory Manager.

The dependency is:

```text
Memory Manager
      │
      │ Statistics
      ▼
AI Scheduler
      │
      ▼
Scheduling Decision
```

---

# 47. Scheduling and AI Agent Failure

If an AI agent crashes:

```text
Agent Running
     ↓
Failure
     ↓
Process State Updated
     ↓
Scheduler removes failed agent
     ↓
Another process selected
```

After checkpoint recovery:

```text
Agent Recovered
     ↓
Process → READY
     ↓
Scheduler
     ↓
Agent may receive CPU
```

The Scheduler therefore interacts with the Checkpoint Manager indirectly through the Process Manager and AI Agent Manager.

---

# 48. Scheduling Demonstration

The final demonstration shall include:

```text
1. Create 3 normal processes.
2. Create 2 AI agents.
3. Place all workloads in the Ready Queue.
4. Select FCFS.
5. Execute simulation.
6. Record metrics.
7. Repeat using Round Robin.
8. Repeat using Priority Scheduling.
9. Repeat using AI Adaptive Scheduling.
10. Display comparison.
```

---

# 49. Expected Scheduling Visualization

The dashboard should show:

```text
┌──────────────────────────────────────┐
│        CURRENT SCHEDULER             │
│                                      │
│ Algorithm: AI Adaptive               │
│ Running: Research Agent              │
│ Queue: P2 → P3 → Coding Agent        │
│                                      │
│ Decision Score: 82.5                 │
│ Reason: High waiting + high urgency  │
└──────────────────────────────────────┘
```

And:

```text
CPU Timeline

P1 |----|
P2      |--|
A1          |----|
P3                |--|
A2                    |----|
```

---

# 50. Success Criteria

The Scheduling subsystem is complete when:

* A Ready Queue is maintained.
* FCFS works.
* Round Robin works.
* Priority Scheduling works.
* AI Adaptive Scheduling works.
* Context switching occurs correctly.
* Timer interrupts can trigger preemption.
* Time quantum works for Round Robin.
* Scheduling metrics are calculated.
* Gantt-style execution history is generated.
* Scheduling decisions are logged.
* AI scheduling decisions are explainable.
* Starvation/waiting time can be observed.
* Algorithms can be compared using the same workload.
* Results are displayed in the dashboard.

---

# 51. Summary

The Scheduler determines which process receives the simulated CPU.

The overall flow is:

```text
                 Ready Queue
                      │
                      ▼
                  Scheduler
                      │
       ┌──────────────┼──────────────┐
       │              │              │
       ▼              ▼              ▼
      FCFS        Round Robin     Priority
       │              │              │
       └──────────────┼──────────────┘
                      │
                      ▼
               AI Adaptive
                      │
                      ▼
              Selected Process
                      │
                      ▼
              Process Manager
                      │
                      ▼
                     CPU
```

Classical algorithms provide the OS foundation.

The AI Adaptive Scheduler extends this foundation by using multiple workload characteristics to make explainable scheduling decisions.

The next subsystem is **`09_Synchronization.md`**, which will define mutexes, semaphores, critical sections, race conditions, shared resources, synchronization scenarios, and how processes/AI agents safely access shared state.
