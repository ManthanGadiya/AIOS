# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 10. Inter-Process Communication

## 1. Purpose

This document defines the Inter-Process Communication (IPC) subsystem of AIOS.

The IPC subsystem allows processes and AI agents to exchange information through controlled OS-managed communication mechanisms.

The subsystem covers:

* Process-to-process communication
* Agent-to-agent communication
* Message passing
* Message queues
* Send and receive operations
* Blocking communication
* Non-blocking communication
* IPC synchronization
* IPC permissions
* IPC events
* Communication monitoring

---

# 2. IPC Concept

Processes normally operate in isolated memory spaces.

Therefore, one process should not directly access another process's memory.

Instead, processes communicate through an OS-managed IPC mechanism.

```text
P1
 │
 │ Message
 ▼
IPC Manager
 │
 ▼
P2
```

For AI agents:

```text
Research Agent
      │
      │ Message
      ▼
 IPC Manager
      │
      ▼
 Coding Agent
```

---

# 3. Why IPC Is Required

AIOS contains multiple independent workloads.

Example:

```text
P1
P2
P3
Research Agent
Coding Agent
```

Some workloads may need to exchange information.

Examples:

```text
P1 → P2
Research Agent → Coding Agent
Coding Agent → Research Agent
Process → AI Agent
AI Agent → Process
```

IPC provides a controlled way to perform this communication.

---

# 4. IPC Architecture

The IPC Manager acts as the central communication layer.

```text
                  IPC Manager
                       │
       ┌───────────────┼────────────────┐
       │               │                │
       ▼               ▼                ▼
 Message Queue    Send / Receive    IPC Control
       │               │                │
       └───────────────┼────────────────┘
                       │
                       ▼
              Processes / Agents
```

---

# 5. Message Passing

AIOS shall primarily use message passing for IPC.

A sender creates a message and sends it through the IPC Manager.

```text
Sender
  │
  ▼
send()
  │
  ▼
IPC Manager
  │
  ▼
Message Queue
  │
  ▼
Receiver
  │
  ▼
receive()
```

The sender does not directly modify the receiver's memory.

---

# 6. IPC Message

Every IPC message shall contain information identifying the communication.

Conceptually:

```text
Message
├── Message ID
├── Sender PID
├── Receiver PID
├── Message Type
├── Payload
└── Timestamp / Simulation Cycle
```

Example:

```text
{
    "message_id": 101,
    "sender_pid": 4,
    "receiver_pid": 5,
    "type": "TASK_REQUEST",
    "payload": "Process this dataset"
}
```

The exact internal C++ structure will be finalized during implementation.

---

# 7. Sender and Receiver

The process generating the message is the sender.

The process receiving the message is the receiver.

Example:

```text
Research Agent
     │
     │ Sender
     ▼
IPC Manager
     │
     ▼
Coding Agent
     │
     │ Receiver
```

The sender and receiver are identified using their PIDs or agent identifiers.

---

# 8. Message Queue

AIOS shall use message queues to temporarily store IPC messages.

Example:

```text
Message Queue

┌──────────────────────────┐
│ Message 101              │
├──────────────────────────┤
│ Message 102              │
├──────────────────────────┤
│ Message 103              │
└──────────────────────────┘
```

Messages remain in the queue until the receiving process retrieves them.

---

# 9. Queue Ownership

A message queue may be associated with:

* A specific process
* A pair of communicating processes
* A communication channel
* A shared service

For the initial implementation, a queue may be associated with a receiver or communication channel.

Example:

```text
Queue Q1
Receiver = Coding Agent
```

Messages intended for that receiver are placed in Q1.

---

# 10. Send Operation

A process uses a send operation to transmit a message.

Conceptually:

```text
send(receiver, message)
```

Flow:

```text
Sender
  │
  ▼
Create Message
  │
  ▼
IPC Manager
  │
  ▼
Validate Request
  │
  ▼
Place Message in Queue
  │
  ▼
Notify Receiver
```

---

# 11. Receive Operation

A receiver retrieves a message using:

```text
receive()
```

Flow:

```text
Receiver
   │
   ▼
receive()
   │
   ▼
IPC Manager
   │
   ▼
Check Message Queue
   │
   ├── Message Available
   │       │
   │       ▼
   │    Return Message
   │
   └── Empty
           │
           ▼
       WAITING
```

---

# 12. Blocking Receive

A blocking receive causes a process to wait when no message is available.

Example:

```text
P2
 │
 ▼
receive()
 │
 ▼
Queue Empty
 │
 ▼
P2 → WAITING
```

When a message arrives:

```text
Message Arrives
      │
      ▼
Wake P2
      │
      ▼
P2 → READY
```

The Scheduler can then select P2.

---

# 13. Non-Blocking Receive

A non-blocking receive does not make the process wait.

If the queue is empty:

```text
receive()
   │
   ▼
Queue Empty
   │
   ▼
Return "NO_MESSAGE"
```

The process continues execution.

This mode may be useful for agents that periodically check for incoming tasks.

---

# 14. Blocking vs Non-Blocking IPC

| Feature               | Blocking         | Non-Blocking        |
| --------------------- | ---------------- | ------------------- |
| Queue empty           | Process waits    | Returns immediately |
| Process state         | WAITING          | Continues           |
| Useful for            | Request/response | Periodic polling    |
| Scheduler interaction | High             | Lower               |

Both behaviours should be supported if practical.

---

# 15. IPC System Calls

IPC operations shall be accessible through system calls.

Possible system calls:

```text
IPC_SEND
IPC_RECEIVE
```

Example:

```text
SYSCALL IPC_SEND
```

Flow:

```text
Process
   │
   ▼
SYSCALL IPC_SEND
   │
   ▼
Interrupt
   │
   ▼
System Call Manager
   │
   ▼
IPC Manager
   │
   ▼
Message Queue
```

---

# 16. IPC Send Request

A send request may contain:

```text
IPC Send Request
├── Sender PID
├── Receiver PID
├── Message Type
└── Payload
```

Example:

```text
{
    "sender": 4,
    "receiver": 5,
    "type": "DATA",
    "payload": "Dataset ready"
}
```

---

# 17. IPC Receive Request

A receive request may contain:

```text
IPC Receive Request
├── Receiver PID
├── Queue ID
└── Blocking Mode
```

Example:

```text
{
    "receiver": 5,
    "queue": "Q5",
    "blocking": true
}
```

---

# 18. IPC Synchronization

Message queues are shared OS-managed resources.

Multiple processes may attempt to access the same queue.

Therefore, queue operations may require synchronization.

```text
Process A ──┐
Process B ──┼──► IPC Queue
Process C ──┘
```

The Synchronization Manager may protect the queue using a mutex.

```text
LOCK Queue
    ↓
Modify Queue
    ↓
UNLOCK Queue
```

This prevents concurrent queue modification from producing inconsistent state.

---

# 19. IPC and Process States

IPC can cause process state changes.

Example:

```text
P1 → send()
       │
       ▼
Message Queue
       │
       ▼
P2 → READY
```

For a blocking receive:

```text
P2 → receive()
       │
       ▼
Queue Empty
       │
       ▼
P2 → WAITING
```

When P1 sends a message:

```text
Message Arrives
      │
      ▼
Wake P2
      │
      ▼
P2 → READY
```

---

# 20. IPC and Scheduler

The IPC Manager does not directly schedule processes.

Instead:

```text
IPC Manager
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

This maintains the modular architecture.

---

# 21. IPC and Interrupts

IPC operations may generate OS events.

For example:

```text
IPC_SEND
   │
   ▼
Message Queued
   │
   ▼
Receiver Notification
```

If the receiver is waiting:

```text
Message Arrival
      │
      ▼
IPC Event
      │
      ▼
Wake Receiver
```

The implementation may represent these as internal OS events rather than hardware-style interrupts.

---

# 22. Agent-to-Agent Communication

One of the main purposes of IPC in AIOS is communication between AI agents.

Example:

```text
Research Agent
      │
      │ "Research completed"
      ▼
IPC Manager
      │
      ▼
Coding Agent
```

The Coding Agent can then use the received information to continue its task.

---

# 23. AI Agent Communication Example

Suppose the two agents are:

```text
Agent A = Research Agent
Agent B = Coding Agent
```

Research Agent completes research.

It sends:

```text
{
    "type": "RESEARCH_RESULT",
    "payload": "Dataset analysis completed"
}
```

Flow:

```text
Research Agent
      │
      ▼
IPC_SEND
      │
      ▼
IPC Manager
      │
      ▼
Coding Agent Queue
      │
      ▼
Coding Agent
      │
      ▼
IPC_RECEIVE
```

---

# 24. Agent Task Delegation

IPC can also be used for task delegation.

Example:

```text
Research Agent
      │
      │ "Analyze dataset"
      ▼
Coding Agent
```

Message:

```text
{
    "type": "TASK_REQUEST",
    "payload": "Analyze dataset X"
}
```

Coding Agent receives the task and begins execution.

---

# 25. Agent Response

The receiving agent can send a response.

```text
Research Agent
      │
      │ TASK_REQUEST
      ▼
Coding Agent
      │
      │ TASK_RESULT
      ▼
Research Agent
```

This creates a request-response communication pattern.

---

# 26. Agent Communication Flow

Complete example:

```text
┌─────────────────┐
│ Research Agent  │
└────────┬────────┘
         │
         │ TASK_REQUEST
         ▼
┌─────────────────┐
│   IPC Manager   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Coding Agent    │
└────────┬────────┘
         │
         │ TASK_RESULT
         ▼
┌─────────────────┐
│   IPC Manager   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Research Agent  │
└─────────────────┘
```

---

# 27. IPC Permissions

Not every process should necessarily communicate with every other process.

AIOS may use IPC permissions.

Example:

```text
Research Agent
Allowed:
    Coding Agent

Denied:
    Process 3
```

A send request can therefore be checked before the message is delivered.

```text
Sender
  │
  ▼
IPC Request
  │
  ▼
Permission Check
  │
  ├── Allowed ──► Queue
  │
  └── Denied ───► IPC Error
```

---

# 28. IPC Permission Example

Suppose:

```text
Agent A → Agent B = Allowed
Agent A → Agent C = Denied
```

Request:

```text
Agent A → Agent C
```

Result:

```text
IPC Request
    │
    ▼
Permission Manager
    │
    ▼
DENIED
```

The message is not placed into the receiver's queue.

---

# 29. Message Types

AIOS shall support different message types.

Initial types:

```text
TASK_REQUEST
TASK_RESULT
DATA
STATUS
NOTIFICATION
ERROR
CONTROL
```

AI agents may use additional semantic message types.

Example:

```text
RESEARCH_RESULT
CODE_REQUEST
CODE_RESULT
```

---

# 30. Message Priority

Messages may optionally have priorities.

Example:

```text
Message 1 → Priority 2
Message 2 → Priority 8
Message 3 → Priority 4
```

If priority-based message queues are implemented:

```text
Priority 8
   ↓
Priority 4
   ↓
Priority 2
```

The initial implementation may use FIFO ordering to keep IPC behaviour simple.

Priority-based message handling can be added as an extension.

---

# 31. Message Size

AIOS shall define a maximum simulated message size.

Example:

```text
MAX_MESSAGE_SIZE = 256 units
```

If a message exceeds the allowed size:

```text
Send Request
    │
    ▼
Size Check
    │
    ▼
Too Large
    │
    ▼
IPC Error
```

The exact limit can be configured.

---

# 32. IPC Queue Capacity

Message queues may have limited capacity.

Example:

```text
Queue Capacity = 3

Q1:
[M1]
[M2]
[M3]
```

If another message arrives:

```text
M4 → Queue Full
```

The system may either:

```text
Reject M4
```

or:

```text
Block Sender
```

The initial implementation should use a clearly documented policy.

Recommended initial policy:

> Reject the message and return an IPC error when the queue is full.

---

# 33. IPC Error Handling

Possible IPC errors include:

```text
Invalid Receiver
Invalid Sender
Permission Denied
Queue Full
Queue Not Found
Message Too Large
Invalid Message
```

Example:

```text
P1 → send(P9)
```

If P9 does not exist:

```text
INVALID_RECEIVER
```

---

# 34. IPC Data Structures

The IPC Manager shall maintain structures conceptually similar to:

```text
Message
├── Message ID
├── Sender PID
├── Receiver PID
├── Type
├── Payload
└── Timestamp
```

And:

```text
Message Queue
├── Queue ID
├── Receiver / Channel
├── Capacity
├── Messages
└── Waiting Processes
```

---

# 35. IPC Manager Responsibilities

The IPC Manager is responsible for:

* Creating communication channels.
* Creating and storing messages.
* Validating sender and receiver.
* Checking IPC permissions.
* Adding messages to queues.
* Removing messages from queues.
* Supporting blocking receive.
* Supporting non-blocking receive.
* Handling queue capacity.
* Generating IPC events.
* Tracking communication statistics.

---

# 36. IPC Manager Non-Responsibilities

The IPC Manager shall not:

* Execute CPU instructions.
* Select the next CPU process.
* Directly allocate memory.
* Directly manipulate page tables.
* Directly manage AI tool permissions.
* Directly perform synchronization primitives.

Instead, it communicates with:

```text
Process Manager
Scheduler
Synchronization Manager
AI Agent Manager
System Call Manager
```

---

# 37. IPC Interfaces

The IPC Manager should expose functionality conceptually similar to:

```text
IPCManager
│
├── createChannel()
├── sendMessage()
├── receiveMessage()
├── hasMessage()
├── getQueue()
├── closeChannel()
├── getMessages()
└── getStatistics()
```

The exact C++ method signatures will be finalized during implementation.

---

# 38. IPC Events

The IPC Manager shall generate events such as:

```text
IPC_CHANNEL_CREATED
IPC_SEND
IPC_RECEIVE
IPC_MESSAGE_QUEUED
IPC_MESSAGE_DELIVERED
IPC_QUEUE_FULL
IPC_PERMISSION_DENIED
IPC_ERROR
```

Example:

```text
{
    "type": "IPC_SEND",
    "sender": 4,
    "receiver": 5,
    "message_type": "TASK_REQUEST"
}
```

---

# 39. IPC Statistics

AIOS shall collect communication statistics.

Examples:

```text
Messages Sent
Messages Received
Messages Pending
Queue Utilization
Failed Sends
Permission Denials
Average Message Wait Time
```

Per-process statistics may include:

```text
Messages Sent
Messages Received
IPC Blocks
IPC Errors
```

---

# 40. IPC Dashboard

The frontend should display communication activity.

Example:

```text
┌──────────────────────────────────────┐
│ IPC MONITOR                          │
├──────────────────────────────────────┤
│ Research Agent                       │
│       │                              │
│       │ TASK_REQUEST                 │
│       ▼                              │
│ Coding Agent                         │
└──────────────────────────────────────┘
```

Message queue:

```text
┌──────────────┬──────────────┬────────┐
│ Queue        │ Messages     │ Status │
├──────────────┼──────────────┼────────┤
│ Agent-5      │ 2            │ READY  │
│ Process-2    │ 0            │ EMPTY  │
└──────────────┴──────────────┴────────┘
```

---

# 41. IPC Event Timeline

Example event log:

```text
[Cycle 30]
Research Agent → IPC_SEND

[Cycle 31]
Message queued for Coding Agent

[Cycle 32]
Coding Agent → IPC_RECEIVE

[Cycle 33]
Message delivered
```

This should be visible in the event log.

---

# 42. IPC and Synchronization

IPC queues may be protected using synchronization mechanisms.

Example:

```text
Sender
  │
  ▼
LOCK Queue
  │
  ▼
Add Message
  │
  ▼
UNLOCK Queue
```

For receiving:

```text
Receiver
  │
  ▼
LOCK Queue
  │
  ▼
Remove Message
  │
  ▼
UNLOCK Queue
```

This prevents inconsistent queue state.

---

# 43. IPC and Memory

Message payloads must be represented within the simulator's memory model.

The IPC Manager should not bypass the OS memory abstraction.

Conceptually:

```text
Process
   │
   ▼
Message Payload
   │
   ▼
IPC Manager
   │
   ▼
OS-managed Message Storage
```

The exact implementation may use simulator-managed data structures rather than mapping every message byte into simulated RAM.

The important requirement is that IPC remains an OS-managed mechanism.

---

# 44. IPC and Process Termination

If a process terminates while messages are pending:

```text
Process P2
    │
    ▼
TERMINATED
```

The IPC Manager must handle messages addressed to P2 according to system policy.

Possible initial policy:

> Pending messages for a terminated receiver are discarded and an IPC event is generated.

Example:

```text
P1 → Message → P2
                  │
                  ▼
              P2 TERMINATED
                  │
                  ▼
          Message Discarded
```

---

# 45. IPC and Agent Failure

If an AI agent crashes:

```text
Agent A
   │
   ▼
CRASH
```

The IPC Manager must update its communication state.

Messages waiting for the failed agent may be:

```text
Discarded
```

or retained for recovery.

For the initial project, the recommended policy is:

> Pending messages remain associated with the agent's checkpoint/recovery state only if the recovery scenario explicitly requires them; otherwise, pending messages are discarded.

This keeps failure handling deterministic.

---

# 46. IPC and Checkpoint Recovery

Suppose:

```text
Research Agent
     │
     ▼
Checkpoint
```

Then:

```text
Agent Crash
     │
     ▼
Recovery
     │
     ▼
Agent READY
```

After recovery, the agent can establish IPC communication again.

The IPC Manager should not automatically execute stale messages unless the recovery policy explicitly permits it.

---

# 47. Complete IPC Demonstration

The recommended final demonstration is:

```text
1. Create Research Agent.
2. Create Coding Agent.
3. Start both agents.
4. Research Agent generates a result.
5. Research Agent sends TASK_RESULT.
6. IPC Manager validates the request.
7. Message enters Coding Agent's queue.
8. Coding Agent receives the message.
9. Coding Agent processes it.
10. Coding Agent sends a response.
11. Research Agent receives the response.
12. Display communication on the dashboard.
```

---

# 48. IPC Demonstration Flow

```text
┌─────────────────┐
│ Research Agent  │
└────────┬────────┘
         │
         │ TASK_RESULT
         ▼
┌─────────────────┐
│   IPC Manager   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Message Queue   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Coding Agent    │
└────────┬────────┘
         │
         │ TASK_ACK
         ▼
┌─────────────────┐
│   IPC Manager   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Research Agent  │
└─────────────────┘
```

---

# 49. IPC Demonstration with Blocking

An additional demonstration can show blocking communication.

```text
Coding Agent
     │
     ▼
IPC_RECEIVE
     │
     ▼
Queue Empty
     │
     ▼
WAITING
```

Then:

```text
Research Agent
     │
     ▼
IPC_SEND
     │
     ▼
Message Queue
     │
     ▼
Wake Coding Agent
     │
     ▼
Coding Agent → READY
```

This demonstrates the connection between:

```text
IPC
+
Process Management
+
Scheduling
```

---

# 50. Success Criteria

The IPC subsystem is complete when:

* Processes can send messages.
* Processes can receive messages.
* AI agents can communicate.
* Message queues work.
* Blocking receive works.
* Non-blocking receive works.
* Queue capacity is enforced.
* IPC permissions can be checked.
* Invalid communication requests are handled.
* IPC operations are synchronized where required.
* IPC events are logged.
* IPC statistics are collected.
* Agent-to-agent communication is visible in the dashboard.
* IPC works with process state transitions.
* IPC works with the Scheduler and Process Manager.

---

# 51. Summary

IPC provides controlled communication between independent processes and AI agents.

The central flow is:

```text
Sender
   │
   ▼
IPC_SEND
   │
   ▼
System Call / OS
   │
   ▼
IPC Manager
   │
   ▼
Message Queue
   │
   ▼
Receiver
   │
   ▼
IPC_RECEIVE
```

For AIOS, IPC is particularly important because it allows the two simulated AI agents to cooperate without directly sharing their private process memory.

The IPC subsystem therefore connects:

```text
Process Manager
       │
       ▼
     IPC
       │
 ┌─────┴─────┐
 ▼           ▼
Process    AI Agent
       │
       ▼
Synchronization
```

This completes the **Inter-Process Communication** portion of the Stage III requirements.

The next document is **`11_IO_Buffering_Spooling.md`**, which will define simulated I/O devices, I/O requests, device queues, buffering, spooling, I/O interrupts, blocking processes, and how these behaviours appear in the GUI.
