# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 11. I/O Handling, Buffering and Spooling

## 1. Purpose

This document defines how AIOS simulates Input/Output operations.

The I/O subsystem is responsible for:

* Simulated I/O devices
* I/O requests
* Device queues
* I/O scheduling
* Process blocking during I/O
* I/O completion
* I/O interrupts
* Buffering
* Spooling
* I/O error handling
* I/O statistics
* I/O visualization

AIOS does not directly control physical hardware.

All devices and operations are simulated inside the OS simulator.

---

# 2. I/O Concept

I/O allows a process to communicate with devices or simulated external resources.

Examples:

```text
Keyboard
Printer
Disk
Network
File System
```

In AIOS these are represented as simulated devices.

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
Simulated Device
   │
   ▼
I/O Completion
```

---

# 3. Simulated I/O Devices

AIOS shall initially support a small set of simulated devices.

Recommended devices:

```text
I/O Devices
│
├── Disk
├── Printer
├── Keyboard
└── Network
```

The implementation does not need to communicate with actual physical devices.

Each device exists as an object inside the simulator.

---

# 4. I/O Device Representation

A simulated device may contain:

```text
Device
├── Device ID
├── Device Type
├── State
├── Current Request
├── Request Queue
├── Processing Time
└── Error State
```

Example:

```text
Device ID: D1
Type: Printer
State: BUSY
Current Process: P2
Queue: P3, A1
```

---

# 5. Device States

A simulated device can have states such as:

```text
AVAILABLE
BUSY
ERROR
```

Example:

```text
Printer
   │
   ├── AVAILABLE
   │
   ├── BUSY
   │
   └── ERROR
```

When a device finishes an operation:

```text
BUSY → AVAILABLE
```

---

# 6. I/O Request

A process generates an I/O request when it needs a device.

Conceptually:

```text
I/O Request
├── Request ID
├── PID
├── Device ID
├── Operation
├── Data / Buffer Reference
├── Requested At
└── Status
```

Example:

```text
Request ID: IO-101
PID: 2
Device: Printer
Operation: WRITE
Status: QUEUED
```

---

# 7. I/O Operations

Initial simulated operations may include:

```text
READ
WRITE
```

Examples:

```text
Keyboard → READ
Disk     → READ
Printer  → WRITE
Network  → READ / WRITE
```

The simulator can later add additional operations if required.

---

# 8. I/O Request Flow

The general I/O flow is:

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
Device Queue
   │
   ▼
Simulated Device
   │
   ▼
I/O Processing
   │
   ▼
I/O Completion
   │
   ▼
Interrupt
   │
   ▼
Process → READY
```

---

# 9. I/O and Process State

A process performing blocking I/O normally cannot continue execution until the operation completes.

Therefore:

```text
RUNNING
   │
   ▼
I/O Request
   │
   ▼
WAITING
```

While the process is waiting:

```text
CPU
 │
 ▼
Scheduler
 │
 ▼
Another READY Process
```

This demonstrates multiprogramming.

---

# 10. I/O Blocking Example

Suppose P1 is running.

```text
P1
 │
 ▼
REQUEST READ
 │
 ▼
Keyboard
 │
 ▼
I/O Manager
 │
 ▼
P1 → WAITING
```

The Scheduler can then select P2:

```text
P1 WAITING
     │
     │
     ▼
   CPU → P2
```

When the keyboard operation completes:

```text
Keyboard
   │
   ▼
I/O Complete
   │
   ▼
Interrupt
   │
   ▼
P1 → READY
```

---

# 11. I/O Device Queue

If multiple processes request the same device, requests are placed into a queue.

Example:

```text
Printer Queue

┌──────┬──────┬──────┐
│ P1   │ P3   │ A1   │
└──────┴──────┴──────┘
```

The printer processes one request at a time in the basic implementation.

---

# 12. I/O Queue Management

The I/O Manager shall:

1. Receive I/O requests.
2. Validate the request.
3. Identify the requested device.
4. Add the request to the device queue.
5. Block the requesting process if required.
6. Start the request when the device becomes available.
7. Simulate processing time.
8. Generate completion.
9. Wake the waiting process.

---

# 13. I/O Completion

After the simulated device finishes its operation:

```text
Device
   │
   ▼
Operation Complete
   │
   ▼
I/O Manager
   │
   ▼
Generate Completion Event
   │
   ▼
Interrupt Manager
   │
   ▼
Wake Process
   │
   ▼
READY
```

The Scheduler can then select the process again.

---

# 14. I/O Interrupt

I/O completion should generate a simulated interrupt.

Example:

```text
Printer
   │
   ▼
Operation Complete
   │
   ▼
I/O Interrupt
   │
   ▼
Interrupt Manager
   │
   ▼
I/O Completion Handler
```

This connects the I/O subsystem with the Stage II interrupt requirement.

---

# 15. I/O Interrupt Handling

The interrupt flow is:

```text
I/O Device
    │
    ▼
Completion
    │
    ▼
Interrupt Generated
    │
    ▼
Interrupt Manager
    │
    ▼
Identify Device
    │
    ▼
Identify Request
    │
    ▼
Complete Request
    │
    ▼
Wake Process
    │
    ▼
Process → READY
```

---

# 16. I/O and Scheduler

I/O completion can affect scheduling.

Example:

```text
P1 → WAITING
P2 → RUNNING
P3 → READY
```

P1's I/O completes:

```text
P1 → READY
```

The Scheduler now has:

```text
Ready Queue

P3
P1
```

Depending on the selected scheduling algorithm, P1 may receive CPU time.

---

# 17. I/O and Multiprogramming

I/O blocking demonstrates why multiprogramming is useful.

Without multiprogramming:

```text
P1 waits for I/O
      │
      ▼
CPU remains unused
```

With multiprogramming:

```text
P1 → WAITING
       │
       ▼
     CPU
       │
       ▼
      P2
```

Therefore, another process can use the CPU while P1 waits for I/O.

---

# 18. I/O Buffer

A buffer is a temporary storage area used while data is being transferred between a process and a device.

Conceptually:

```text
Process
   │
   ▼
Buffer
   │
   ▼
Device
```

The buffer can temporarily hold data during an I/O operation.

---

# 19. Why Buffering Is Required

CPU and I/O devices may operate at different speeds.

For example:

```text
CPU
Fast
 │
 ▼
Buffer
 │
 ▼
Printer
Slow
```

Instead of requiring the CPU to wait for every small device operation, data can be placed in a buffer.

---

# 20. Input Buffering

For input:

```text
Device
   │
   ▼
Input Buffer
   │
   ▼
Process
```

Example:

```text
Keyboard
   │
   ▼
Input Buffer
   │
   ▼
Process
```

The keyboard data can wait in the buffer until the process consumes it.

---

# 21. Output Buffering

For output:

```text
Process
   │
   ▼
Output Buffer
   │
   ▼
Device
```

Example:

```text
Process
   │
   ▼
Output Buffer
   │
   ▼
Printer
```

The process can place output into the buffer while the printer processes it.

---

# 22. Buffer States

A buffer may be represented as:

```text
Buffer
├── Buffer ID
├── Capacity
├── Current Size
├── Data
└── State
```

Possible states:

```text
EMPTY
PARTIAL
FULL
```

Example:

```text
Capacity = 10

Current Data = 6

State = PARTIAL
```

---

# 23. Buffer Full

If an output buffer becomes full:

```text
Process
   │
   ▼
Output Buffer
   │
   ▼
FULL
```

The system must apply a defined policy.

Possible behaviour:

```text
Block Process
or
Return Buffer-Full Error
```

For the initial implementation, the recommended behaviour is:

> Block the process when a blocking output operation cannot place additional data into a full buffer.

---

# 24. Buffer Empty

If a process requests input while the input buffer is empty:

```text
Process
   │
   ▼
READ
   │
   ▼
Input Buffer
   │
   ▼
EMPTY
```

For blocking input:

```text
Process → WAITING
```

When input arrives:

```text
Device
   │
   ▼
Input Buffer
   │
   ▼
Wake Process
```

---

# 25. Double Buffering

AIOS may optionally simulate double buffering.

Two buffers are used:

```text
Buffer A
Buffer B
```

While one buffer is being processed:

```text
Buffer A → Device
```

the other can receive data:

```text
Process → Buffer B
```

Then they switch.

```text
A → Device
B → Fill

then

B → Device
A → Fill
```

This is an optional extension and is not required for the minimum project.

---

# 26. Spooling

Spooling stands for **Simultaneous Peripheral Operations On-Line**.

In the simulator, spooling represents storing I/O requests in an intermediate queue so that multiple processes can submit requests to a device that can process them one at a time.

Example:

```text
P1 ──┐
P2 ──┤
P3 ──┼──► Spool Queue ──► Printer
A1 ──┤
A2 ──┘
```

The spool queue decouples the processes from the slower device.

---

# 27. Why Spooling Is Useful

Consider a printer.

Only one print operation can be performed at a time.

However, many processes may want to print.

Without spooling:

```text
P1 → Printer
P2 → WAIT
P3 → WAIT
```

With spooling:

```text
P1 ──┐
P2 ──┤
P3 ──┼──► Print Queue → Printer
A1 ──┤
A2 ──┘
```

Processes can submit jobs to the spool queue.

---

# 28. Spool Queue

The spool queue contains pending I/O jobs.

Example:

```text
Printer Spool

┌────────┬────────┬────────┬────────┐
│ Job 1  │ Job 2  │ Job 3  │ Job 4  │
└────────┴────────┴────────┴────────┘
```

Each job may contain:

```text
Spool Job
├── Job ID
├── Owner PID
├── Device
├── Operation
├── Data
└── Status
```

---

# 29. Spooling Flow

```text
Process
   │
   ▼
I/O Request
   │
   ▼
Spool Manager
   │
   ▼
Spool Queue
   │
   ▼
Device Available?
   │
   ├── No → Wait
   │
   └── Yes
         │
         ▼
      Device
         │
         ▼
     Completion
```

---

# 30. Spooling vs Buffering

These concepts should not be confused.

### Buffering

Temporarily holds data during a transfer.

```text
Process
  ↕
Buffer
  ↕
Device
```

### Spooling

Queues complete I/O jobs for a device.

```text
Processes
    │
    ▼
Spool Queue
    │
    ▼
Device
```

Simple distinction:

> **Buffer = temporary data storage.**

> **Spool = queue of pending jobs.**

---

# 31. I/O Scheduling

When multiple requests are waiting for a device, the I/O Manager needs a policy for selecting the next request.

The initial implementation shall use:

```text
FIFO
```

Example:

```text
Request Order:

IO1 → IO2 → IO3

Execution:

IO1 → IO2 → IO3
```

Additional I/O scheduling algorithms may be added later if required.

---

# 32. Device Request States

An I/O request may have states:

```text
CREATED
   ↓
QUEUED
   ↓
PROCESSING
   ↓
COMPLETED
```

If an error occurs:

```text
PROCESSING
   ↓
ERROR
```

Example:

```text
IO-101
Status = PROCESSING
```

---

# 33. I/O Error Handling

The I/O subsystem shall simulate possible errors.

Examples:

```text
Invalid Device
Device Busy
Device Error
Buffer Full
Buffer Empty
Invalid Request
Queue Full
```

Example:

```text
Process
   │
   ▼
Request Device D5
   │
   ▼
D5 does not exist
   │
   ▼
I/O ERROR
```

The Error Manager handles the resulting system event.

---

# 34. Device Failure

A simulated device can optionally enter an error state.

Example:

```text
Printer
   │
   ▼
BUSY
   │
   ▼
Simulated Failure
   │
   ▼
ERROR
```

Requests waiting for the device can remain queued or fail according to the configured policy.

The initial implementation may use:

> Current request fails, queued requests remain pending until the device is restored.

---

# 35. I/O Recovery

When a failed device is restored:

```text
Device ERROR
     │
     ▼
Device Recovery
     │
     ▼
AVAILABLE
     │
     ▼
Process Next Queue Entry
```

A recovery event should be generated.

---

# 36. AI Agent I/O

AI agents use the same I/O subsystem as normal processes.

Example:

```text
Research Agent
      │
      ▼
READ Dataset
      │
      ▼
Disk
```

or:

```text
Coding Agent
      │
      ▼
WRITE Output
      │
      ▼
Disk
```

The AI layer does not directly control simulated devices.

---

# 37. AI Agent and Spooling

AI agents can submit I/O jobs to spool queues.

Example:

```text
Research Agent ──┐
Coding Agent ────┼──► Disk Spool
Process 2 ───────┘
```

The I/O Manager processes these jobs according to its configured policy.

---

# 38. I/O and Synchronization

A shared device may require synchronization.

Example:

```text
P1 ──┐
P2 ──┼──► Printer
A1 ──┘
```

The I/O Manager can use synchronization mechanisms to protect device state.

For example:

```text
Printer Mutex
```

ensures that only the appropriate operation modifies the device state at a time.

---

# 39. I/O and IPC

IPC can be used to request or report I/O operations.

Example:

```text
Agent A
   │
   │ I/O Request
   ▼
IPC Manager
   │
   ▼
I/O Manager
   │
   ▼
Device
```

However, normal I/O requests should primarily use the I/O subsystem rather than bypassing it through IPC.

---

# 40. I/O Events

The I/O Manager shall generate events such as:

```text
IO_REQUEST
IO_QUEUED
IO_STARTED
IO_COMPLETED
IO_BLOCKED
IO_INTERRUPT
IO_ERROR
DEVICE_AVAILABLE
DEVICE_BUSY
DEVICE_FAILURE
DEVICE_RECOVERED
BUFFER_FULL
BUFFER_EMPTY
SPOOL_JOB_CREATED
SPOOL_JOB_COMPLETED
```

Example:

```text
{
    "type": "IO_REQUEST",
    "pid": 2,
    "device": "D1",
    "operation": "WRITE"
}
```

---

# 41. I/O Statistics

AIOS shall collect:

```text
Total I/O Requests
Completed Requests
Failed Requests
Average I/O Wait Time
Device Utilization
Queue Length
Buffer Utilization
Spool Queue Length
I/O Interrupt Count
```

Per process:

```text
PID
I/O Requests
I/O Wait Time
Completed I/O
Failed I/O
```

---

# 42. I/O Dashboard

The frontend should visualize devices.

Example:

```text
┌─────────────────────────────────────┐
│ I/O DEVICES                         │
├──────────┬───────────┬──────────────┤
│ Device   │ State     │ Queue        │
├──────────┼───────────┼──────────────┤
│ Disk     │ BUSY      │ P2 → A1      │
│ Printer  │ AVAILABLE │ P3           │
│ Keyboard │ AVAILABLE │ -            │
│ Network  │ BUSY      │ A2           │
└──────────┴───────────┴──────────────┘
```

---

# 43. Buffer Visualization

The dashboard may show:

```text
┌──────────────────────────────────────┐
│ OUTPUT BUFFER                        │
│                                      │
│ [██████████████░░░░░░] 70%           │
│                                      │
│ Capacity: 20                         │
│ Used: 14                              │
│ Free: 6                               │
└──────────────────────────────────────┘
```

The exact visual design will be implemented later.

---

# 44. Spooling Visualization

Example:

```text
┌──────────────────────────────────────┐
│ PRINTER SPOOL                        │
├──────────────────────────────────────┤
│ Job 101 → P1                         │
│ Job 102 → Research Agent             │
│ Job 103 → P3                         │
│ Job 104 → Coding Agent               │
└──────────────────────────────────────┘

                │
                ▼

             PRINTER
             [BUSY]
```

This provides a clear visual demonstration of spooling.

---

# 45. I/O Timeline

The event log should display:

```text
[Cycle 15]
P1 → Disk READ

[Cycle 16]
P1 → WAITING

[Cycle 20]
Disk → READ COMPLETE

[Cycle 21]
I/O INTERRUPT

[Cycle 22]
P1 → READY

[Cycle 23]
Scheduler → P1 selected
```

This connects:

```text
I/O
+
Interrupt
+
Process Management
+
Scheduling
```

---

# 46. Complete I/O Demonstration

Recommended final demo:

```text
1. Create P1, P2 and AI agents.
2. Start P1.
3. P1 requests disk I/O.
4. P1 becomes WAITING.
5. Scheduler selects another READY process.
6. Disk processes the request.
7. Disk generates completion.
8. Interrupt is generated.
9. P1 becomes READY.
10. Scheduler eventually selects P1.
```

Then demonstrate buffering:

```text
11. Process writes data to output buffer.
12. Buffer fills.
13. Device consumes buffered data.
14. Buffer space becomes available.
```

Then demonstrate spooling:

```text
15. Multiple processes submit printer jobs.
16. Jobs enter spool queue.
17. Printer processes jobs sequentially.
18. Dashboard displays the spool queue.
```

---

# 47. Complete I/O Architecture

```text
                    I/O Manager
                         │
          ┌──────────────┼──────────────┐
          │              │              │
          ▼              ▼              ▼
       Devices        Buffers        Spooling
          │              │              │
          └──────────────┼──────────────┘
                         │
                         ▼
                   I/O Requests
                         │
                         ▼
                  Process Manager
                         │
                         ▼
                  Process State
                         │
                         ▼
                     Scheduler
```

---

# 48. I/O Interrupt Architecture

```text
Simulated Device
       │
       ▼
I/O Operation Complete
       │
       ▼
Interrupt Generated
       │
       ▼
Interrupt Manager
       │
       ▼
I/O Completion Handler
       │
       ▼
Process Manager
       │
       ▼
WAITING → READY
       │
       ▼
Scheduler
```

---

# 49. I/O Manager Interface

The I/O Manager should expose functionality conceptually similar to:

```text
IOManager
│
├── registerDevice()
├── requestIO()
├── startIO()
├── completeIO()
├── cancelIO()
├── getDevice()
├── getDeviceQueue()
├── createBuffer()
├── writeBuffer()
├── readBuffer()
├── createSpoolJob()
├── processSpoolQueue()
└── getStatistics()
```

The exact C++ interfaces will be finalized during implementation.

---

# 50. Success Criteria

The I/O subsystem is complete when:

* Simulated devices can be created.
* Processes can request I/O.
* I/O requests enter device queues.
* Processes block during blocking I/O.
* I/O operations have simulated processing time.
* I/O completion is generated.
* I/O completion generates an interrupt.
* Waiting processes can become READY.
* Buffering can be demonstrated.
* Input buffering works.
* Output buffering works.
* Spooling can be demonstrated.
* Multiple jobs can enter a spool queue.
* I/O errors are detected.
* Device failures can be simulated.
* I/O statistics are collected.
* I/O state is visible in the dashboard.

---

# 51. Summary

The I/O subsystem simulates how an operating system manages devices and handles processes waiting for I/O.

The core flow is:

```
                Process
                   │
                   ▼
              I/O Request
                   │
                   ▼
               I/O Manager
                   │
      ┌────────────┼────────────┐
      ▼            ▼            ▼
   Device       Buffer       Spool
      │            │            │
      └────────────┼────────────┘
                   │
                   ▼
              I/O Complete
                   │
                   ▼
             I/O Interrupt
                   │
                   ▼
            Interrupt Manager
                   │
                   ▼
          Process → READY
                   │
                   ▼
               Scheduler
                   │
                   ▼
                  CPU
```

Buffering temporarily stores data during transfer.

Spooling queues complete I/O jobs for devices that process requests sequentially.

Together, these mechanisms complete the **I/O Handling, Spooling and Buffering** portion of the project requirements.
