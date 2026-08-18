# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 9. Process Synchronization

## 1. Purpose

This document defines the synchronization mechanisms used by AIOS to coordinate processes and AI agents that access shared resources.

The Synchronization subsystem is responsible for:

* Critical sections
* Shared resources
* Race-condition prevention
* Mutexes
* Semaphores
* Resource locking
* Process blocking and waking
* Synchronization events
* Agent synchronization

Synchronization is required because multiple workloads may execute concurrently in the multiprogramming environment.

---

# 2. Why Synchronization Is Required

AIOS may have multiple processes and AI agents:

```text
P1
P2
P3
Agent A
Agent B
```

These workloads may access shared resources such as:

```text
Shared Memory
IPC Queues
I/O Devices
Knowledge Bus
System Resources
```

Without synchronization, two workloads may attempt to modify the same resource simultaneously.

This can produce incorrect or unpredictable results.

---

# 3. Shared Resource

A shared resource is a resource that can be accessed by multiple processes.

Examples:

```text
Shared Resource
│
├── Shared Memory
├── IPC Queue
├── I/O Device
├── Knowledge Bus
└── Resource Table
```

Example:

```text
P1 ─────┐
        │
P2 ─────┼──► Shared Resource
        │
A1 ─────┘
```

The OS must control access to such resources.

---

# 4. Critical Section

A critical section is the part of a process where it accesses or modifies a shared resource.

Example:

```text
Process
   │
   ▼
Normal Code
   │
   ▼
Critical Section
   │
   ▼
Normal Code
```

The objective is:

> Only an allowed process should execute the critical section at a time.

---

# 5. Example of a Race Condition

Suppose two processes increment the same shared variable.

Initial value:

```text
Counter = 10
```

Both execute:

```text
Counter = Counter + 1
```

Without synchronization:

```text
P1 reads 10
P2 reads 10

P1 writes 11
P2 writes 11
```

Expected:

```text
12
```

Actual:

```text
11
```

This is a race condition.

---

# 6. Race Condition

A race condition occurs when the result of execution depends on the timing or ordering of concurrent operations on shared data.

Conceptually:

```text
P1 ──────► Read ─────► Write
             │
P2 ──────► Read ─────► Write
```

If both processes access the resource without proper synchronization, the final state may be incorrect.

---

# 7. Synchronization Goal

AIOS synchronization mechanisms should provide:

### Mutual Exclusion

Only one process can enter a protected critical section at a time.

### Progress

If the resource is available, a waiting process should eventually be able to access it.

### Bounded Waiting

A process should not wait indefinitely when competing for a resource.

These concepts will be demonstrated through the simulator.

---

# 8. Mutex

A mutex is a synchronization mechanism used to provide mutual exclusion.

Conceptually:

```text
Mutex
│
├── LOCKED
└── UNLOCKED
```

Only one process can hold the mutex at a time.

---

# 9. Mutex Operation

A process performs:

```text
LOCK
   ↓
Critical Section
   ↓
UNLOCK
```

Example:

```text
P1
 │
 ▼
LOCK Resource
 │
 ▼
Critical Section
 │
 ▼
UNLOCK Resource
```

---

# 10. Mutex Contention

Suppose P1 already owns the mutex.

```text
Mutex = LOCKED
Owner = P1
```

P2 requests the same mutex:

```text
P2
 │
 ▼
LOCK
 │
 ▼
Mutex already locked
 │
 ▼
P2 → WAITING
```

When P1 releases it:

```text
P1
 │
 ▼
UNLOCK
 │
 ▼
Mutex Available
 │
 ▼
Wake P2
 │
 ▼
P2 → READY
```

---

# 11. Mutex State

AIOS shall maintain information about each mutex.

Conceptually:

```text
Mutex
├── Mutex ID
├── State
├── Owner PID
└── Waiting Processes
```

Example:

```text
Mutex ID: M1
State: LOCKED
Owner: PID 2
Waiting: PID 3, PID 4
```

---

# 12. Semaphore

A semaphore is a synchronization mechanism based on a counter.

Unlike a mutex, a semaphore can allow multiple processes to access a resource depending on its value.

Example:

```text
Semaphore = 3
```

This means up to three resource units may be available.

---

# 13. Semaphore Operations

AIOS shall model two basic operations:

```text
WAIT()
SIGNAL()
```

Conceptually:

```text
WAIT()
 ↓
Decrease semaphore count

SIGNAL()
 ↓
Increase semaphore count
```

---

# 14. Semaphore Example

Suppose:

```text
Semaphore = 2
```

Three processes request the resource.

```text
P1 → WAIT → Semaphore = 1
P2 → WAIT → Semaphore = 0
P3 → WAIT → BLOCKED
```

When P1 releases the resource:

```text
P1 → SIGNAL
        ↓
Semaphore = 1
        ↓
P3 can proceed
```

---

# 15. Mutex vs Semaphore

| Feature      | Mutex               | Semaphore                    |
| ------------ | ------------------- | ---------------------------- |
| Main purpose | Mutual exclusion    | Resource coordination        |
| Count        | Binary              | Integer                      |
| Ownership    | Has an owner        | No strict owner requirement  |
| Example      | Shared data         | Multiple resource slots      |
| Access       | Usually one process | Can allow multiple processes |

For the AIOS implementation:

```text
Mutex → Protect shared critical section
Semaphore → Control limited resource availability
```

---

# 16. Synchronization Architecture

The Synchronization Manager sits between processes and shared resources.

```text
                 Synchronization Manager
                         │
          ┌──────────────┴──────────────┐
          │                             │
          ▼                             ▼
        Mutex                       Semaphore
          │                             │
          └──────────────┬──────────────┘
                         │
                         ▼
                  Shared Resource
```

---

# 17. Synchronization Manager

The Synchronization Manager is responsible for:

* Creating synchronization objects
* Acquiring locks
* Releasing locks
* Managing semaphore counts
* Blocking processes
* Waking waiting processes
* Detecting invalid synchronization operations
* Generating synchronization events

---

# 18. Process Interaction

A process requests a synchronization object through the OS.

Conceptually:

```text
Process
   │
   ▼
System Call
   │
   ▼
Synchronization Manager
   │
   ▼
Mutex / Semaphore
   │
   ▼
Shared Resource
```

The process does not directly modify the synchronization object's internal state.

---

# 19. Synchronization Through System Calls

AIOS can expose synchronization operations through system calls.

Possible operations:

```text
LOCK
UNLOCK
WAIT
SIGNAL
```

For example:

```text
SYSCALL LOCK(M1)
```

Flow:

```text
Process
   │
   ▼
SYSCALL
   │
   ▼
Interrupt
   │
   ▼
System Call Manager
   │
   ▼
Synchronization Manager
   │
   ▼
Mutex
```

---

# 20. Process State During Synchronization

If a process cannot acquire the required resource, it enters the WAITING state.

```text
READY
  │
  ▼
RUNNING
  │
  ▼
Request Resource
  │
  ▼
Resource Busy
  │
  ▼
WAITING
```

When the resource becomes available:

```text
Resource Available
      │
      ▼
Waiting Process
      │
      ▼
READY
```

The Scheduler can later give the process CPU time.

---

# 21. Synchronization and Scheduler

The Synchronization Manager does not directly select which process runs next.

Instead:

```text
Synchronization Manager
          │
          ▼
Process State Change
          │
          ▼
Process Manager
          │
          ▼
Ready Queue
          │
          ▼
Scheduler
```

This preserves separation of responsibilities.

---

# 22. Critical Section Example

Suppose P1 and P2 both want to modify a shared counter.

Without synchronization:

```text
P1 ───────► Shared Counter ◄─────── P2
```

With a mutex:

```text
P1
 │
 ▼
LOCK
 │
 ▼
Shared Counter
 │
 ▼
UNLOCK
 │
 ▼
P2
 │
 ▼
LOCK
 │
 ▼
Shared Counter
 │
 ▼
UNLOCK
```

Only one process accesses the critical section at a time.

---

# 23. Mutex Demonstration Scenario

Initial state:

```text
Mutex M1 = UNLOCKED
Counter = 0
```

P1 and P2 both execute:

```text
LOCK M1
Counter = Counter + 1
UNLOCK M1
```

Execution:

```text
P1 → LOCK M1
      ↓
P1 → Counter = 1
      ↓
P1 → UNLOCK M1
      ↓
P2 → LOCK M1
      ↓
P2 → Counter = 2
      ↓
P2 → UNLOCK M1
```

Final value:

```text
Counter = 2
```

---

# 24. Semaphore Demonstration Scenario

Suppose AIOS simulates three identical I/O resources.

```text
Semaphore S1 = 3
```

Five processes request access.

```text
P1 → Resource
P2 → Resource
P3 → Resource
P4 → WAITING
P5 → WAITING
```

Semaphore value:

```text
0
```

When P2 finishes:

```text
P2 → SIGNAL
```

Semaphore becomes:

```text
1
```

One waiting process can proceed.

---

# 25. AI Agent Synchronization

AI agents are also subject to synchronization.

Example:

```text
Research Agent ──┐
                 ├──► Knowledge Bus
Coding Agent ────┘
```

If both agents modify a shared knowledge structure simultaneously, synchronization may be required.

The AI Agent Manager therefore uses OS synchronization mechanisms rather than implementing separate uncontrolled locking.

---

# 26. Shared Knowledge Bus

The project's shared knowledge bus can be treated as a shared resource.

Conceptually:

```text
Agent A
   │
   ▼
LOCK
   │
   ▼
Knowledge Bus
   │
   ▼
Update Knowledge
   │
   ▼
UNLOCK
```

Another agent attempting access while the lock is held must wait.

---

# 27. IPC and Synchronization

IPC and synchronization may work together.

Example:

```text
Agent A
   │
   │ Send Message
   ▼
IPC Queue
   │
   ▼
Agent B
```

The IPC queue may require synchronization if multiple producers or consumers access it.

Conceptually:

```text
Producer 1 ──┐
Producer 2 ──┼──► Shared IPC Queue
Producer 3 ──┘
```

A mutex or semaphore can protect the queue.

Detailed IPC behaviour is defined in:

```text
10_IPC.md
```

---

# 28. Synchronization and I/O

I/O resources may also require synchronization.

Example:

```text
P1 ──┐
P2 ──┼──► Simulated Printer
P3 ──┘
```

If only one process can use the printer at a time:

```text
Printer Mutex = M1
```

The process must acquire M1 before using the printer.

---

# 29. Deadlock

Deadlock occurs when processes wait indefinitely for resources held by one another.

Example:

```text
P1 holds M1
P2 holds M2

P1 waits for M2
P2 waits for M1
```

Result:

```text
P1 ──waiting──► M2
 ▲             │
 │             ▼
M1 ◄──waiting── P2
```

Neither process can continue.

---

# 30. Deadlock Demonstration

AIOS may include a controlled deadlock scenario.

Example:

```text
P1:
LOCK M1
LOCK M2

P2:
LOCK M2
LOCK M1
```

If execution is interleaved:

```text
P1 → LOCK M1
P2 → LOCK M2
P1 → WAIT M2
P2 → WAIT M1
```

The system reaches:

```text
DEADLOCK
```

This can be displayed in the dashboard.

---

# 31. Deadlock Detection

The initial implementation may use a simple resource-wait graph.

Example:

```text
P1 → M2
M2 → P2

P2 → M1
M1 → P1
```

A cycle indicates a potential deadlock.

Conceptually:

```text
P1 → M2 → P2 → M1 → P1
```

Cycle detected:

```text
DEADLOCK
```

Deadlock recovery is optional unless required by the project demonstration.

---

# 32. Synchronization Error Handling

The Synchronization Manager should detect invalid operations.

Examples:

```text
Unlocking an unlocked mutex
Unlocking a mutex owned by another process
Invalid mutex ID
Invalid semaphore ID
Invalid semaphore operation
```

Example:

```text
P2
 │
 ▼
UNLOCK M1
 │
 ▼
P2 is not owner
 │
 ▼
Synchronization Error
```

The Error Manager handles the resulting error according to system policy.

---

# 33. Synchronization Events

The Synchronization Manager shall generate events such as:

```text
MUTEX_CREATED
MUTEX_LOCKED
MUTEX_UNLOCKED
PROCESS_BLOCKED_ON_MUTEX
PROCESS_WAKEUP
SEMAPHORE_CREATED
SEMAPHORE_WAIT
SEMAPHORE_SIGNAL
DEADLOCK_DETECTED
SYNCHRONIZATION_ERROR
```

Example:

```text
{
    "type": "MUTEX_LOCKED",
    "mutex_id": "M1",
    "pid": 2
}
```

---

# 34. Synchronization Visualization

The dashboard should display synchronization state.

Example:

```text
┌──────────────────────────────────────┐
│ SYNCHRONIZATION                      │
├──────────┬─────────┬─────────────────┤
│ Mutex    │ State   │ Owner           │
├──────────┼─────────┼─────────────────┤
│ M1       │ LOCKED  │ P2              │
│ M2       │ FREE    │ -               │
└──────────┴─────────┴─────────────────┘
```

Semaphore:

```text
┌────────────┬───────┬────────────────┐
│ Semaphore  │ Value │ Waiting        │
├────────────┼───────┼────────────────┤
│ S1         │ 2     │ P3             │
└────────────┴───────┴────────────────┘
```

---

# 35. Synchronization Timeline

The event log should show synchronization operations.

Example:

```text
[Cycle 20]
P1 → LOCK M1

[Cycle 21]
P1 → Enter Critical Section

[Cycle 23]
P2 → LOCK M1

[Cycle 23]
P2 → WAITING

[Cycle 25]
P1 → UNLOCK M1

[Cycle 26]
P2 → READY
```

This makes synchronization behaviour easy to demonstrate.

---

# 36. Synchronization Data Structures

The Synchronization Manager shall maintain structures conceptually similar to:

```text
Mutex
├── ID
├── State
├── Owner PID
└── Waiting Queue
```

and:

```text
Semaphore
├── ID
├── Count
└── Waiting Queue
```

---

# 37. Synchronization Manager Interface

Conceptually:

```text
SynchronizationManager
│
├── createMutex()
├── lockMutex()
├── unlockMutex()
├── createSemaphore()
├── waitSemaphore()
├── signalSemaphore()
├── getMutexState()
├── getSemaphoreState()
└── detectDeadlock()
```

The exact C++ interfaces will be finalized during implementation.

---

# 38. Synchronization Configuration

Synchronization objects may be defined through configuration.

Example:

```text
{
    "mutexes": [
        {
            "id": "M1",
            "resource": "knowledge_bus"
        }
    ],
    "semaphores": [
        {
            "id": "S1",
            "resource": "io_device",
            "count": 3
        }
    ]
}
```

This keeps synchronization configuration separate from implementation code.

---

# 39. Complete Synchronization Flow

The complete flow is:

```text
Process / AI Agent
        │
        ▼
Request Shared Resource
        │
        ▼
System Call
        │
        ▼
Interrupt Manager
        │
        ▼
Synchronization Manager
        │
        ▼
Check Mutex / Semaphore
        │
     ┌──┴──┐
     │     │
 Available Busy
     │     │
     ▼     ▼
 Grant   WAITING
 Access    │
     │     │
     ▼     │
Critical   │
Section    │
     │     │
     ▼     │
 Release   │
     │     │
     └──┬──┘
        ▼
 Waiting Process
        │
        ▼
       READY
```

---

# 40. Synchronization and the AI Scheduler

Synchronization state can affect scheduling indirectly.

For example:

```text
P1 → WAITING on M1
P2 → RUNNING
P3 → READY
```

The Scheduler must not select P1 while it remains blocked.

When P1 obtains the resource:

```text
P1 → READY
```

it becomes eligible for scheduling again.

The AI Scheduler may also use waiting time as one of its scheduling factors.

---

# 41. Synchronization and Checkpoint Recovery

If an AI agent crashes while holding a synchronization resource, the OS must prevent the resource from remaining permanently locked.

Example:

```text
Agent A
   │
   ▼
LOCK M1
   │
   ▼
Agent A crashes
```

The AI Agent Manager / Process Manager must trigger cleanup.

Conceptually:

```text
Agent Crash
    │
    ▼
Process Cleanup
    │
    ▼
Release Owned Resources
    │
    ▼
Wake Waiting Processes
```

This prevents a crashed agent from creating an unnecessary permanent lock.

---

# 42. Demonstration Scenario

The final demonstration should include at least one synchronization example.

Recommended scenario:

```text
1. Create P1 and P2.
2. Give both access to a shared counter.
3. Run without synchronization.
4. Demonstrate inconsistent result.
5. Reset simulation.
6. Run using Mutex.
7. Show serialized access.
8. Compare results.
```

Expected visualization:

```text
Without Mutex:

P1 ── Read ─────── Write
P2 ───── Read ─────── Write

Race Condition


With Mutex:

P1 ── LOCK ── Read ── Write ── UNLOCK
P2                         └── LOCK ── Read ── Write ── UNLOCK
```

---

# 43. AI Agent Synchronization Demonstration

A second demonstration may use the two AI agents.

Example:

```text
Research Agent
      │
      ▼
Knowledge Bus
      ▲
      │
Coding Agent
```

Both agents attempt to update shared knowledge.

The synchronization mechanism ensures:

```text
Only one update operation
at a time.
```

Event log:

```text
[Cycle 40]
Research Agent → LOCK Knowledge Bus

[Cycle 41]
Research Agent → UPDATE

[Cycle 42]
Coding Agent → WAIT

[Cycle 43]
Research Agent → UNLOCK

[Cycle 44]
Coding Agent → READY

[Cycle 45]
Coding Agent → LOCK
```

---

# 44. Success Criteria

The Synchronization subsystem is complete when:

* Shared resources can be represented.
* Critical sections can be demonstrated.
* Race conditions can be simulated.
* Mutexes can be created.
* Mutex locking works.
* Mutex unlocking works.
* Processes can block when a mutex is unavailable.
* Waiting processes can be awakened.
* Semaphores can be created.
* Semaphore wait/signal operations work.
* Processes can block on semaphore availability.
* Synchronization errors are detected.
* Synchronization events are logged.
* AI agents can use synchronization mechanisms.
* Controlled deadlock can be demonstrated or detected.
* Synchronization state is visible in the dashboard.

---

# 45. Summary

Synchronization protects shared resources when multiple processes or AI agents operate concurrently.

The central flow is:

```text
          Multiple Workloads
                 │
                 ▼
          Shared Resource
                 │
                 ▼
       Synchronization Manager
                 │
          ┌──────┴──────┐
          │             │
          ▼             ▼
        Mutex       Semaphore
          │             │
          └──────┬──────┘
                 │
                 ▼
          Resource Access
```

Mutexes are primarily used for mutual exclusion.

Semaphores are used to coordinate access to a limited number of resource units.

The synchronization subsystem integrates with:

```text
Process Manager
Scheduler
Interrupt Manager
IPC Manager
I/O Manager
AI Agent Manager
```

This completes the synchronization portion of the Stage III requirements.

The next document is **`10_IPC.md`**, which will define how processes and AI agents exchange messages through the OS, including message queues, send/receive operations, blocking communication, and the AI-agent communication demo.
