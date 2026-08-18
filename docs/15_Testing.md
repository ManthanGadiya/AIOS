# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 15. Testing

## 1. Purpose

This document defines the testing strategy for AIOS.

Testing ensures that:

* Individual OS modules work correctly.
* OS modules interact correctly.
* Required Stage I, Stage II, and Stage III concepts are implemented.
* AI-agent functionality works correctly.
* The GUI accurately represents the internal simulator state.
* Demonstration scenarios are repeatable.
* Errors and failures are handled correctly.

The testing process focuses on the behavior of the simulated operating system rather than testing the GUI alone.

---

# 2. Testing Strategy

AIOS shall use multiple levels of testing:

```text
Unit Testing
     ↓
Module Testing
     ↓
Integration Testing
     ↓
System Testing
     ↓
Scenario Testing
     ↓
GUI Testing
     ↓
Final Demonstration Testing
```

---

# 3. Unit Testing

Unit testing verifies individual components independently.

Examples:

```text
CPU
Memory Manager
Page Table
Process Manager
Scheduler
Interrupt Manager
IPC Manager
Synchronization Manager
I/O Manager
AI Agent Manager
Checkpoint Manager
```

Each component should be tested with controlled inputs.

---

# 4. CPU Testing

## Test CPU Initialization

### Input

Create a new CPU.

### Expected Result

```text
PC = Initial PC
IR = Empty
ACC = 0
MAR = 0
MBR = 0
FLAGS = Default
```

---

## Test Instruction Fetch

### Input

Place an instruction in memory.

### Expected Result

```text
PC
 ↓
MAR
 ↓
MBR
 ↓
IR
```

The instruction should be loaded into the Instruction Register.

---

## Test Instruction Execution

Test supported instructions individually.

Example:

```text
LOAD
STORE
ADD
SUB
JUMP
HALT
```

Expected result:

The CPU state should change according to the instruction.

---

# 5. System Call Testing

## Test Supervisor Call

### Input

A running process executes:

```text
SYSCALL
```

### Expected Flow

```text
Process
   ↓
System Call
   ↓
Interrupt
   ↓
System Call Handler
   ↓
OS Service
   ↓
Return to Process
```

### Expected Result

The process should continue execution after the system call is serviced.

---

# 6. Process Management Testing

## Test Process Creation

Create:

```text
P1
```

Expected:

```text
PID assigned
PCB created
Process state = NEW
```

---

## Test Process Admission

After loading the process into memory:

```text
NEW → READY
```

Expected result:

The process appears in the Ready Queue.

---

## Test Process Termination

When the process executes HALT:

```text
RUNNING → TERMINATED
```

Expected result:

The process is removed from the Ready Queue and CPU.

---

# 7. PCB Testing

Each process should have a Process Control Block.

Test that the PCB contains the required information.

Example:

```text
PID
Process State
Program Counter
CPU Registers
Priority
Memory Information
Scheduling Information
```

Expected result:

PCB information remains consistent with the actual process state.

---

# 8. Multiprogramming Testing

## Test Multiple Processes

Create:

```text
P1
P2
P3
```

Load them into memory.

Expected result:

```text
P1 → READY
P2 → READY
P3 → READY
```

Multiple processes should coexist in the simulated system.

---

# 9. Context Switching Testing

## Test

Run:

```text
P1
```

Generate a timer interrupt.

Expected:

```text
P1 → READY
P2 → RUNNING
```

The CPU context of P1 must be saved.

The CPU context of P2 must be restored.

---

# 10. Scheduler Testing

Each scheduling algorithm should be tested separately.

Algorithms:

```text
FCFS
Round Robin
Priority
AI Adaptive
```

---

# 11. FCFS Testing

Example:

```text
P1
P2
P3
```

Arrival order:

```text
P1 → P2 → P3
```

Expected execution order:

```text
P1 → P2 → P3
```

---

# 12. Round Robin Testing

Use:

```text
Time Quantum = 2
```

Processes:

```text
P1
P2
P3
```

Expected behavior:

```text
P1 → P2 → P3 → P1 → P2 → ...
```

until processes complete.

The Scheduler should preempt a process when its time quantum expires.

---

# 13. Priority Scheduling Testing

Assign:

```text
P1 → Priority 1
P2 → Priority 3
P3 → Priority 2
```

The configured priority policy should determine which process is selected first.

The exact meaning of higher/lower priority must remain consistent throughout the simulator.

---

# 14. AI Adaptive Scheduler Testing

The AI Scheduler should evaluate:

```text
Priority
Waiting Time
Urgency
Resource Usage
```

Example:

```text
P1 → Score 40
P2 → Score 55
A1 → Score 82
A2 → Score 63
```

Expected:

```text
A1
```

is selected if the configured policy selects the highest score.

The Scheduler should also provide an explanation for its decision.

---

# 15. Scheduler Metrics Testing

For every scheduling algorithm, calculate:

```text
Average Waiting Time
Average Turnaround Time
Average Response Time
CPU Utilization
Context Switch Count
Throughput
```

The same workload should be used when comparing algorithms.

---

# 16. Paging Testing

## Test Page Table

Create:

```text
Page 0
Page 1
Page 2
Page 3
```

Map pages to physical frames.

Expected:

```text
Page → Frame
```

mapping is correctly maintained.

---

# 17. Address Translation Testing

Given:

```text
Logical Address
```

the Memory Manager should determine:

```text
Page Number
Offset
```

and use the Page Table to determine:

```text
Frame Number
```

The resulting physical address should be correct.

---

# 18. Page Fault Testing

Create a page table where:

```text
Page 2 → Invalid
```

Access Page 2.

Expected:

```text
Memory Access
      ↓
Page Not Present
      ↓
Page Fault
```

The Page Fault counter should increase.

---

# 19. Page Fault Handling Testing

After a page fault:

```text
Page Fault
   ↓
Interrupt
   ↓
Memory Manager
   ↓
Find Free Frame
   ↓
Load Page
   ↓
Update Page Table
   ↓
Resume Process
```

Expected:

```text
Valid = 1
```

for the loaded page.

---

# 20. Virtual Memory Testing

The simulator should allow a process's logical address space to be larger than its currently allocated physical frames.

Example:

```text
Logical Pages = 8
Physical Frames available to process = 4
```

Expected:

Only required pages need to be present in physical memory.

Accessing an absent page should produce a page fault.

---

# 21. Swap Testing

When no suitable free frame is available:

```text
Page Replacement
      ↓
Victim Page
      ↓
Swap Out
      ↓
Required Page
      ↓
Swap In
```

Expected:

The page table and frame ownership are updated correctly.

---

# 22. Error Handling Testing

The simulator should handle invalid operations safely.

Examples:

```text
Invalid PID
Invalid Memory Address
Invalid Page
Invalid Instruction
Invalid System Call
Invalid IPC Receiver
Permission Denied
Resource Limit Exceeded
```

The simulator should not crash.

Instead, it should generate an appropriate error event.

---

# 23. Interrupt Testing

Test each supported interrupt.

```text
Timer Interrupt
I/O Interrupt
Page Fault Interrupt
System Call Interrupt
IPC Event
```

For each interrupt verify:

```text
Interrupt Generated
        ↓
Interrupt Recognized
        ↓
Handler Selected
        ↓
Interrupt Serviced
        ↓
Normal Execution Resumed
```

---

# 24. Interrupt Priority Testing

If multiple interrupts are pending, verify that the configured interrupt policy is followed.

Example:

```text
Timer Interrupt
I/O Interrupt
Page Fault
```

The Interrupt Manager should process them according to the defined priority/order.

---

# 25. Synchronization Testing

## Test Mutex

Create:

```text
P1
P2
```

Both attempt to access the same critical section.

Expected:

```text
P1 → LOCK
P1 → Critical Section
P1 → UNLOCK

P2 → LOCK
P2 → Critical Section
P2 → UNLOCK
```

Only one process should own the mutex at a time.

---

# 26. Race Condition Testing

Run two processes without synchronization.

Example:

```text
Shared Counter = 0
```

Both execute:

```text
Counter = Counter + 1
```

The simulator should be capable of demonstrating the race condition.

Then repeat using a mutex.

Expected:

The synchronized execution produces the correct protected result.

---

# 27. IPC Testing

## Test Message Send

```text
A1 → A2
```

Send:

```text
"RESEARCH_RESULT"
```

Expected:

The message appears in A2's IPC queue.

---

# 28. IPC Receive Testing

A2 receives the message.

Expected:

```text
Message Queue
      ↓
A2
```

The message should be removed from the queue according to the configured IPC policy.

---

# 29. IPC Blocking Testing

If a process performs a blocking receive while the queue is empty:

```text
RECEIVE
  ↓
Queue Empty
  ↓
WAITING
```

When a message arrives:

```text
Message
  ↓
Queue
  ↓
Wake Process
  ↓
READY
```

---

# 30. Invalid IPC Testing

Attempt to send a message to an invalid process.

Example:

```text
A1 → PID 999
```

Expected:

```text
IPC ERROR
Invalid Receiver
```

The simulator should continue running.

---

# 31. I/O Testing

Test:

```text
Disk
Keyboard
Network
Printer
```

A process requests I/O.

Expected:

```text
RUNNING
   ↓
I/O REQUEST
   ↓
WAITING
```

After completion:

```text
I/O COMPLETE
   ↓
INTERRUPT
   ↓
READY
```

---

# 32. I/O Queue Testing

If a device is busy:

```text
P1 → Disk
P2 → Disk
P3 → Disk
```

Expected:

```text
Disk Queue:
P1
P2
P3
```

Requests should be serviced according to the configured device policy.

---

# 33. Buffering Testing

Create a buffer with:

```text
Capacity = 10
```

Add data.

Verify:

```text
Used Space
Free Space
Buffer State
```

Test:

```text
Empty
Partial
Full
```

states.

---

# 34. Buffer Overflow Testing

Attempt to write more data than the buffer can hold.

Expected behavior:

```text
Buffer Full
      ↓
Additional Write
      ↓
Error / Block / Wait
```

The selected behavior must be consistent with the implementation.

---

# 35. Spooling Testing

Submit multiple jobs to the printer.

Example:

```text
P1 → Job 1
P2 → Job 2
A1 → Job 3
A2 → Job 4
```

Expected:

```text
Spool Queue
Job 1
Job 2
Job 3
Job 4
```

Jobs should be processed sequentially.

---

# 36. AI Agent Testing

Create:

```text
A1 → Research Agent
A2 → Coding Agent
```

Verify:

```text
Unique Agent ID
PID
State
Task
Resource Information
Permissions
Checkpoint Information
```

---

# 37. AI Agent Scheduling Testing

Verify that AI agents participate in normal CPU scheduling.

Example:

```text
P1
P2
A1
A2
```

Expected:

All workloads should be eligible for scheduling according to their state and scheduling policy.

---

# 38. AI Agent IPC Testing

Test:

```text
A1 → A2
```

and:

```text
A2 → A1
```

Expected:

Messages should be delivered through the IPC subsystem rather than through direct shared memory access.

---

# 39. AI Agent Tool Permission Testing

Research Agent:

```text
READ_DATA → Allowed
SEARCH → Allowed
WRITE_CODE → Denied
```

Request:

```text
WRITE_CODE
```

Expected:

```text
Permission Denied
```

The event should be logged.

---

# 40. AI Agent Resource Quota Testing

Configure:

```text
CPU Quota = 20 cycles
Memory Quota = 8 pages
Token Budget = 1000
```

Attempt to exceed a configured quota.

Expected:

```text
Resource Limit Event
```

The configured resource-management policy should then be applied.

---

# 41. Checkpoint Testing

Create a checkpoint while an agent is running.

Example:

```text
A1
 ↓
CP-001
```

Verify that the checkpoint contains the required simulated recovery information.

---

# 42. Agent Failure Testing

Trigger:

```text
SIMULATED_CRASH
```

Expected:

```text
A1
 ↓
FAILED
```

The failure should be recorded in the event log.

---

# 43. Agent Recovery Testing

After failure:

```text
FAILED
   ↓
Checkpoint Restore
   ↓
READY
```

Expected:

The agent becomes eligible for scheduling again.

After the Scheduler selects it:

```text
READY
   ↓
RUNNING
```

---

# 44. Resource Cleanup Testing

When an agent fails, verify that resources owned by the agent are correctly handled.

Test:

```text
CPU
Memory
IPC
Mutex
I/O
```

For example, if the agent owns a mutex when failure occurs, the system should follow the configured cleanup policy and prevent the mutex from remaining permanently locked.

---

# 45. GUI Testing

GUI testing verifies that frontend values match backend state.

For example:

```text
Backend:
A1 = RUNNING
```

Expected GUI:

```text
A1 → RUNNING
```

---

# 46. Process Table GUI Test

When a process is created:

```text
Backend
P1 created
```

Expected GUI:

```text
P1
```

appears in the Process List.

When it terminates:

```text
P1 → TERMINATED
```

the GUI must update accordingly.

---

# 47. CPU GUI Test

When CPU registers change:

```text
PC
IR
ACC
MAR
MBR
FLAGS
```

the CPU panel should update.

The GUI must not display stale register values.

---

# 48. Memory GUI Test

When a page is loaded:

```text
Page 2
 ↓
Frame 7
```

the GUI should update:

```text
Memory Grid
Page Table
Page Fault Count
Event Log
```

---

# 49. Scheduler GUI Test

When the Scheduler selects a process:

```text
A1 → RUNNING
```

the GUI should update:

```text
Process State
CPU Panel
Ready Queue
Scheduler Panel
Event Log
```

---

# 50. IPC GUI Test

When:

```text
A1 → A2
```

sends a message, the GUI should display the message in the IPC Message Queue.

---

# 51. Interrupt GUI Test

When an interrupt is generated:

```text
Timer Interrupt
```

the GUI should show:

```text
Interrupt Panel
Event Log
```

and update its handled/pending status.

---

# 52. WebSocket Testing

Verify that live OS events reach the frontend.

Example:

```text
C++ Engine
     ↓
WebSocket Event
     ↓
React
     ↓
Dashboard Update
```

Test events such as:

```text
PROCESS_CREATED
CONTEXT_SWITCH
PAGE_FAULT
IPC_MESSAGE
IO_COMPLETE
AGENT_FAILURE
AGENT_RECOVERY
```

---

# 53. Backend Disconnection Testing

Temporarily stop the backend connection.

Expected GUI:

```text
OS Engine: Disconnected
```

The GUI should not falsely display new live events while disconnected.

---

# 54. Integration Testing

Integration testing verifies interactions between OS modules.

Important combinations include:

```text
CPU + Process Manager
Scheduler + CPU
Scheduler + Process Manager
Memory + Interrupt Manager
IPC + Process Manager
I/O + Interrupt Manager
AI Agent Manager + Process Manager
AI Scheduler + Process Manager
Checkpoint + Process Manager
```

---

# 55. CPU + Scheduler Integration

Test:

```text
Scheduler selects P1
       ↓
CPU runs P1
       ↓
Timer Interrupt
       ↓
Scheduler selects P2
       ↓
CPU runs P2
```

Expected:

The CPU and Scheduler remain synchronized.

---

# 56. Memory + Interrupt Integration

Test:

```text
Process
   ↓
Memory Access
   ↓
Page Fault
   ↓
Interrupt
   ↓
Memory Manager
   ↓
Page Loaded
   ↓
Process Resumes
```

This is an important integrated Stage II demonstration.

---

# 57. I/O + Interrupt Integration

Test:

```text
Process
   ↓
I/O Request
   ↓
WAITING
   ↓
Device
   ↓
I/O Complete
   ↓
Interrupt
   ↓
READY
```

Expected:

The Scheduler can eventually execute the process again.

---

# 58. IPC + Scheduler Integration

Test:

```text
A2 → RECEIVE
      ↓
Queue Empty
      ↓
A2 WAITING
```

Then:

```text
A1 → SEND
      ↓
Message Available
      ↓
A2 READY
```

Expected:

A2 becomes eligible for scheduling.

---

# 59. AI Agent + Scheduler Integration

Test:

```text
A1
A2
P1
P2
```

The AI Scheduler evaluates all eligible workloads.

Expected:

AI agents are treated as OS-managed workloads rather than bypassing the normal process-management system.

---

# 60. Failure Recovery Integration

Test the complete sequence:

```text
A2 RUNNING
    ↓
Checkpoint
    ↓
Failure
    ↓
FAILED
    ↓
Recovery
    ↓
READY
    ↓
Scheduler
    ↓
RUNNING
```

Expected:

The recovered agent continues through the normal OS execution path.

---

# 61. Full System Integration Test

The complete integrated test should execute:

```text
Create Processes
      ↓
Create AI Agents
      ↓
Load Memory
      ↓
Start Scheduler
      ↓
CPU Execution
      ↓
Context Switch
      ↓
Page Fault
      ↓
Interrupt
      ↓
I/O
      ↓
IPC
      ↓
Synchronization
      ↓
Spooling
      ↓
Agent Failure
      ↓
Checkpoint Recovery
      ↓
Resume Execution
```

Expected:

All subsystems operate together without corrupting the simulator state.

---

# 62. Scenario Testing

Every predefined demonstration scenario should be tested independently.

```text
Basic CPU
System Call
Multiprogramming
Scheduling
Paging
Interrupts
Synchronization
IPC
I/O
Buffering
Spooling
AI Scheduling
Agent Failure
Agent Recovery
Full Demo
```

---

# 63. Regression Testing

Whenever a subsystem is modified, previously working scenarios must be executed again.

Example:

If the Scheduler is modified:

```text
CPU Test
Process Test
Multiprogramming Test
Paging Test
IPC Test
AI Agent Test
Full Demo
```

should be rerun.

This ensures that a new change does not break existing functionality.

---

# 64. Performance Testing

The simulator should be tested with increasing numbers of workloads.

Example:

```text
5 processes
10 processes
20 processes
50 processes
```

Measure:

```text
Simulation Response
GUI Update Performance
Memory Usage
Event Processing
```

The project is a simulator, so performance requirements are moderate rather than production-OS scale.

---

# 65. Stress Testing

Stress the system using:

```text
Many processes
Frequent context switches
Frequent interrupts
Multiple IPC messages
Multiple page faults
Multiple I/O requests
```

Expected:

The simulator should remain stable and maintain valid internal state.

---

# 66. Error Recovery Testing

Test invalid and unexpected operations.

Examples:

```text
Invalid PID
Invalid Page
Invalid Memory Access
Invalid IPC Destination
Permission Denied
Resource Quota Exceeded
Empty Queue
Full Buffer
Agent Failure
```

Expected:

The simulator should report the error and continue whenever the configured policy permits.

---

# 67. State Consistency Testing

At all times verify that related components agree.

Example:

If:

```text
CPU.currentProcess = A1
```

then:

```text
A1.state = RUNNING
```

and no other process should simultaneously be marked as the CPU's active process.

Similarly:

If:

```text
A1.state = READY
```

then A1 should be present in the appropriate Ready Queue.

---

# 68. Memory Consistency Testing

Verify:

```text
Page Table
Frame Table
Process Memory
Swap
```

remain consistent.

For example:

If:

```text
Page 2 → Frame 7
```

then Frame 7 must identify the correct owner/page.

---

# 69. IPC Consistency Testing

If a message is sent:

```text
A1 → A2
```

the event should be reflected consistently in:

```text
IPC Queue
A1 IPC Statistics
A2 IPC Statistics
Event Log
GUI
```

---

# 70. Test Result Format

Each test should be recorded using:

```text
Test ID
Test Name
Input
Expected Result
Actual Result
Status
Notes
```

Example:

```text
Test ID:
MEM-001

Test:
Page Fault

Input:
Access invalid page 2

Expected:
Page fault generated

Actual:
Page fault generated

Status:
PASS
```

---

# 71. Test Categories

Test results should be categorized as:

```text
PASS
FAIL
BLOCKED
NOT IMPLEMENTED
```

A failed test should not be silently ignored.

---

# 72. Critical Tests

The following tests are considered critical:

```text
CPU Execution
System Call
Process Creation
Context Switching
Paging
Page Fault
Interrupt Handling
Scheduling
Synchronization
IPC
I/O
AI Agent Creation
AI Agent Scheduling
Agent Failure
Checkpoint Recovery
Full Integration
```

All critical tests should pass before the final demonstration.

---

# 73. Final Demo Verification Checklist

Before the professor demonstration, verify:

### Stage I

```text
[ ] CPU simulation works
[ ] Registers update
[ ] Instructions execute
[ ] Supervisor call works
[ ] System-call interrupt appears
```

### Stage II

```text
[ ] Paging works
[ ] Page table works
[ ] Page faults work
[ ] Error handling works
[ ] Interrupt generation works
[ ] Interrupt servicing works
[ ] PCB works
```

### Stage III

```text
[ ] Multiple processes run
[ ] Virtual memory works
[ ] Scheduling works
[ ] Context switching works
[ ] Synchronization works
[ ] IPC works
[ ] I/O works
[ ] Buffering works
[ ] Spooling works
```

### AI Layer

```text
[ ] AI agents can be created
[ ] AI agents receive PIDs
[ ] AI agents are scheduled
[ ] AI agents communicate
[ ] Resource quotas work
[ ] Tool permissions work
[ ] Checkpoint works
[ ] Agent failure works
[ ] Recovery works
```

### GUI

```text
[ ] Dashboard works
[ ] Live updates work
[ ] CPU panel works
[ ] Memory panel works
[ ] Scheduler panel works
[ ] IPC panel works
[ ] Interrupt panel works
[ ] I/O panel works
[ ] Event log works
```

---

# 74. Final Acceptance Test

The final acceptance test is the complete integrated demonstration.

### Start

```text
Create 3 processes
Create 2 AI agents
```

### Memory

```text
Load workloads into memory
```

### Scheduling

```text
Start AI Adaptive Scheduler
```

### CPU

```text
Execute workloads
```

### Context Switching

```text
Generate timer interrupt
Switch process
```

### Virtual Memory

```text
Generate page fault
Handle page fault
```

### I/O

```text
Perform disk I/O
Handle I/O interrupt
```

### IPC

```text
A1 → A2
A2 → A1
```

### Synchronization

```text
Lock
Critical Section
Unlock
```

### Spooling

```text
Submit multiple printer jobs
```

### Failure Recovery

```text
Checkpoint A2
Crash A2
Recover A2
Resume A2
```

### Results

```text
Display scheduling comparison
Display system metrics
```

---

# 75. Testing Success Criteria

AIOS will be considered ready for final demonstration when:

1. All critical unit tests pass.
2. All major module integration tests pass.
3. Paging and page faults operate correctly.
4. Interrupts are generated and serviced correctly.
5. Multiple processes can execute through the Scheduler.
6. Synchronization prevents the demonstrated race condition.
7. IPC works between processes and AI agents.
8. I/O, buffering, and spooling operate correctly.
9. AI agents are managed as OS workloads.
10. AI Adaptive scheduling produces explainable decisions.
11. Agent failure can be simulated.
12. Checkpoint recovery works.
13. GUI state matches backend state.
14. Full demonstration scenario completes successfully.
15. The same scenario can be repeated reliably.

---

# 76. Testing Summary

The AIOS testing strategy follows:

```text
             UNIT TESTING
                  ↓
           MODULE TESTING
                  ↓
        INTEGRATION TESTING
                  ↓
          SYSTEM TESTING
                  ↓
         SCENARIO TESTING
                  ↓
             GUI TESTING
                  ↓
        FINAL ACCEPTANCE TEST
```

The final objective is not merely to prove that individual modules work.

The objective is to prove that:

```text
CPU
 │
Process Manager
 │
Scheduler
 │
Memory
 │
Interrupts
 │
Synchronization
 │
IPC
 │
I/O
 │
AI Agent Manager
 │
GUI
```

operate as one integrated operating-system simulation.

---

# 77. Final Principle

The most important testing principle for AIOS is:

> **Every feature shown in the GUI must correspond to a real state or event generated by the C++ OS simulation engine.**

Therefore:

```text
Backend State
      ↓
OS Event
      ↓
WebSocket
      ↓
GUI
      ↓
Visual Representation
```

The GUI must never be used to fake an OS operation.

A successful AIOS demonstration should allow the evaluator to trace an operation from:

```text
User Action
     ↓
OS Component
     ↓
State Change
     ↓
Event
     ↓
GUI
```

and verify that the simulated operating system behaves consistently.
