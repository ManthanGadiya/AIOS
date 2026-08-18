# AIOS: An AI-Powered Multiprogramming Operating System Simulator

# 7. Paging and Virtual Memory

## 1. Purpose

This document defines the memory-management architecture of AIOS.

The Memory Management subsystem is responsible for:

* Logical memory
* Physical memory
* Pages
* Frames
* Page tables
* Address translation
* Memory allocation
* Page faults
* Page replacement
* Swap space
* Virtual memory
* Memory protection
* Memory-related error handling

The implementation shall use paging to simulate virtual memory.

---

# 2. Memory Management Overview

AIOS separates a process's logical memory from physical RAM.

```text
Process
   │
   ▼
Logical Address Space
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
Physical RAM
```

A process operates using logical addresses.

The Memory Manager translates those logical addresses into physical addresses.

---

# 3. Logical and Physical Memory

## 3.1 Logical Memory

Logical memory is the address space visible to a process.

Example:

```text
Process P1

Logical Memory
┌──────────┐
│ Page 0   │
├──────────┤
│ Page 1   │
├──────────┤
│ Page 2   │
├──────────┤
│ Page 3   │
├──────────┤
│ Page 4   │
└──────────┘
```

The process behaves as if it owns this address space.

---

## 3.2 Physical Memory

Physical memory represents the simulated RAM.

RAM is divided into fixed-size frames.

```text
Physical RAM

┌──────────┐
│ Frame 0  │
├──────────┤
│ Frame 1  │
├──────────┤
│ Frame 2  │
├──────────┤
│ Frame 3  │
├──────────┤
│ Frame 4  │
└──────────┘
```

Pages are loaded into available frames.

---

# 4. Pages and Frames

Paging divides memory into equal-sized units.

```text
Logical Memory              Physical Memory

Page 0 ───────────────────► Frame 3
Page 1 ───────────────────► Frame 0
Page 2 ───────────────────► Frame 4
Page 3 ───────────────────► Frame 1
Page 4 ───────────────────► Swap
```

The page size and frame size are identical.

This allows any page to be placed into any available frame.

---

# 5. Page Size

AIOS shall use a fixed page size.

For the initial simulator configuration:

```text
Page Size = 4 memory units
```

The value should be configurable so that different memory configurations can be tested.

For example:

```text
PAGE_SIZE = 4
```

The exact number of pages and frames can also be configured.

---

# 6. Example Memory Configuration

A demonstration configuration may use:

```text
Logical Memory:
20 units

Page Size:
4 units

Number of Pages:
5

Physical RAM:
12 units

Number of Frames:
3
```

Therefore:

```text
Logical Pages
P0
P1
P2
P3
P4

Physical Frames
F0
F1
F2
```

Only three pages can be resident in RAM at one time.

The remaining pages may stay in simulated swap storage.

---

# 7. Page Table

Every process shall have a page table.

The page table maps logical pages to physical frames.

Example:

```text
P1 Page Table

┌──────┬────────┬─────────┐
│ Page │ Frame  │ Present │
├──────┼────────┼─────────┤
│  0   │   2    │   Yes   │
│  1   │   0    │   Yes   │
│  2   │   -    │   No    │
│  3   │   1    │   Yes   │
│  4   │   -    │   No    │
└──────┴────────┴─────────┘
```

The `Present` field indicates whether the page is currently loaded into physical memory.

---

# 8. Page Table Fields

Each page-table entry shall contain information such as:

```text
Page Table Entry
├── Page Number
├── Frame Number
├── Present Bit
├── Valid Bit
├── Reference Bit
└── Modified / Dirty Bit
```

Not every field is required for every page replacement algorithm, but the design shall allow these fields to be represented.

---

# 9. Present Bit

The Present Bit indicates whether a page currently exists in physical RAM.

```text
Present = 1
```

means:

```text
Page is in RAM
```

Whereas:

```text
Present = 0
```

means:

```text
Page is not currently in RAM
```

Example:

```text
Page 2 → Present = 0
```

If the process accesses Page 2, a page fault occurs.

---

# 10. Valid Bit

The Valid Bit indicates whether the page belongs to the valid address space of the process.

This helps distinguish:

```text
Valid page but currently not in RAM
```

from:

```text
Invalid memory access
```

Therefore:

```text
Valid = 1, Present = 0
```

can mean:

> The page is valid but currently resides in swap.

Whereas:

```text
Valid = 0
```

can indicate an invalid memory reference.

---

# 11. Logical Address

A logical address is divided into:

```text
Logical Address
       │
       ├── Page Number
       │
       └── Offset
```

For example, with:

```text
Page Size = 4
```

logical address `10` becomes:

```text
Page Number = 10 / 4 = 2
Offset      = 10 % 4 = 2
```

Therefore:

```text
Logical Address 10
       ↓
Page 2
Offset 2
```

---

# 12. Address Translation

The Memory Manager converts:

```text
Logical Address
       ↓
Page Number + Offset
       ↓
Page Table
       ↓
Frame Number
       ↓
Physical Address
```

Physical address calculation:

```text
Physical Address =
(Frame Number × Page Size) + Offset
```

Example:

```text
Page = 2
Offset = 2

Page Table:
Page 2 → Frame 1

Physical Address =
(1 × 4) + 2
= 6
```

Therefore:

```text
Logical Address 10
        ↓
Physical Address 6
```

---

# 13. Address Translation Example

Suppose:

```text
Page Size = 4
Logical Address = 13
```

Then:

```text
Page = 13 / 4
     = 3

Offset = 13 % 4
       = 1
```

Suppose:

```text
Page 3 → Frame 2
```

Then:

```text
Physical Address =
(2 × 4) + 1
= 9
```

Therefore:

```text
Logical Address 13
        ↓
Page 3, Offset 1
        ↓
Frame 2
        ↓
Physical Address 9
```

---

# 14. Memory Access Flow

Every memory access from the CPU follows:

```text
CPU
 │
 │ Logical Address
 ▼
Memory Manager
 │
 ▼
Extract Page + Offset
 │
 ▼
Page Table Lookup
 │
 ├── Present
 │      │
 │      ▼
 │    Frame
 │      │
 │      ▼
 │ Physical Address
 │
 └── Not Present
        │
        ▼
     Page Fault
```

---

# 15. Page Fault

A page fault occurs when a process accesses a valid page that is not currently loaded into RAM.

Example:

```text
Page Table

Page 0 → Frame 1 → Present
Page 1 → Frame 0 → Present
Page 2 → -       → Not Present
```

If the process accesses Page 2:

```text
CPU
 ↓
Page Table
 ↓
Page 2 not present
 ↓
PAGE FAULT
```

---

# 16. Page Fault Handling

The page fault is handled through the interrupt mechanism.

```text
CPU
 │
 ▼
Memory Access
 │
 ▼
Page Not Present
 │
 ▼
Page Fault Generated
 │
 ▼
Interrupt Manager
 │
 ▼
Page Fault Handler
 │
 ▼
Memory Manager
 │
 ▼
Find Free Frame
 │
 ├── Frame Available
 │
 └── No Frame
       │
       ▼
   Page Replacement
       │
       ▼
   Load Required Page
       │
       ▼
   Update Page Table
       │
       ▼
   Retry Instruction
```

---

# 17. Page Fault and Process State

While a page fault is being serviced, the process may temporarily enter a waiting state.

Conceptually:

```text
RUNNING
   │
   ▼
Page Fault
   │
   ▼
WAITING
   │
   ▼
Page Loaded
   │
   ▼
READY
   │
   ▼
RUNNING
```

The exact scheduling behaviour can depend on whether the simulator treats page loading as an immediate or delayed simulated operation.

---

# 18. Free Frame Allocation

When a page fault occurs, the Memory Manager first checks whether a free frame is available.

```text
Free Frame?
    │
 ┌──┴───┐
Yes     No
 │       │
 ▼       ▼
Load   Page
Page   Replacement
```

If a free frame exists:

```text
Required Page → Free Frame
```

The page table is then updated.

---

# 19. Page Replacement

If all physical frames are occupied, the Memory Manager must select a page to remove.

```text
RAM

Frame 0 → Page 2
Frame 1 → Page 5
Frame 2 → Page 1
```

Suppose Page 4 is required.

There is no free frame.

Therefore:

```text
Page 4
  ↓
Page Replacement
  ↓
Select Victim Page
  ↓
Remove Victim
  ↓
Load Page 4
```

---

# 20. Page Replacement Algorithm

AIOS shall initially support:

```text
FIFO
```

as the baseline page replacement algorithm.

An optional second algorithm may be implemented:

```text
LRU
```

### FIFO

The page that has been in memory for the longest time is selected for replacement.

Example:

```text
Loaded order:

Page 1 → Page 2 → Page 3

If Page 4 is required:

Page 1 is replaced.
```

The replacement policy should be configurable.

---

# 21. Dirty Page

A page may have been modified while in memory.

Such a page is called dirty.

Conceptually:

```text
Dirty = 1
```

means the page contains changes that may need to be written back to swap before replacement.

Example:

```text
Page 2
 ├── Frame 1
 ├── Present = 1
 └── Dirty = 1
```

The initial implementation may simplify write-back behaviour if the demonstration does not require persistent modified pages.

---

# 22. Swap Space

AIOS shall simulate a swap area outside physical RAM.

```text
Physical RAM
┌──────────┐
│ Frame 0  │
│ Frame 1  │
│ Frame 2  │
└──────────┘
      │
      │ Page Replacement
      ▼
Simulated Swap
┌──────────┐
│ Slot 0   │
│ Slot 1   │
│ Slot 2   │
│ Slot 3   │
└──────────┘
```

Swap is represented as simulated storage.

No real disk swapping is performed.

---

# 23. Virtual Memory

Virtual memory allows a process to have a logical address space larger than the currently available physical RAM.

Example:

```text
Process Logical Memory
┌──────────┐
│ Page 0   │ ──► RAM
├──────────┤
│ Page 1   │ ──► RAM
├──────────┤
│ Page 2   │ ──► Swap
├──────────┤
│ Page 3   │ ──► RAM
├──────────┤
│ Page 4   │ ──► Swap
└──────────┘
```

The process can still operate using its logical address space.

When it accesses a page in swap:

```text
Page Fault
   ↓
Page Loaded
   ↓
Process Continues
```

---

# 24. Virtual Memory Example

Suppose:

```text
Logical Memory = 20 units
Physical RAM = 12 units
Page Size = 4 units
```

Then:

```text
Logical Pages = 5
Physical Frames = 3
```

A process may have:

```text
Page 0 → RAM
Page 1 → RAM
Page 2 → Swap
Page 3 → RAM
Page 4 → Swap
```

The process still sees:

```text
Page 0
Page 1
Page 2
Page 3
Page 4
```

as its logical memory.

---

# 25. Memory Protection

AIOS shall prevent one process from accessing another process's logical memory.

Each process has its own page table.

Example:

```text
P1 Page Table
Page 0 → Frame 1
Page 1 → Frame 3

P2 Page Table
Page 0 → Frame 0
Page 1 → Frame 2
```

P1 cannot simply use P2's page-table mappings.

An invalid memory reference shall generate an appropriate memory error.

---

# 26. Page Table per Process

The Memory Manager shall maintain a page table for each process.

Example:

```text
Process Table

P1 → Page Table 1
P2 → Page Table 2
P3 → Page Table 3
A1 → Page Table 4
A2 → Page Table 5
```

When the scheduler changes the running process, the Memory Manager uses the selected process's page table for memory translation.

---

# 27. Context Switching and Memory

A context switch affects memory translation because the new process has a different logical address space.

Example:

```text
P1 Running
   │
   ▼
P1 Page Table Active
   │
   ▼
Context Switch
   │
   ▼
P2 Running
   │
   ▼
P2 Page Table Active
```

The simulator shall ensure that memory references are interpreted using the currently running process's memory mapping.

---

# 28. Memory Allocation

When a process is created, the Process Manager requests memory from the Memory Manager.

```text
Process Manager
      │
      ▼
Memory Request
      │
      ▼
Memory Manager
      │
      ▼
Create Pages
      │
      ▼
Create Page Table
      │
      ▼
Return Memory Information
```

The Memory Manager decides which pages can initially be placed in RAM.

---

# 29. Demand Paging

AIOS shall use demand paging behaviour for the virtual-memory demonstration.

This means pages do not necessarily need to be loaded into RAM before they are accessed.

Instead:

```text
Process Created
      │
      ▼
Page Table Created
      │
      ▼
Process Starts
      │
      ▼
Page Access
      │
      ▼
Page Not Present?
      │
      ▼
Page Fault
      │
      ▼
Load Page
```

This makes page-fault behaviour easy to demonstrate.

---

# 30. Memory State Representation

The Memory Manager shall maintain a representation similar to:

```text
Physical Memory

┌────────┬──────────┬──────────┬─────────┐
│ Frame  │ Page     │ PID      │ Status  │
├────────┼──────────┼──────────┼─────────┤
│ F0     │ P1-Page2 │ PID 1    │ Used    │
│ F1     │ P2-Page0 │ PID 2    │ Used    │
│ F2     │ P4-Page1 │ PID 4    │ Used    │
└────────┴──────────┴──────────┴─────────┘
```

The frontend can display this information visually.

---

# 31. Page Table Visualization

The dashboard should be able to display:

```text
PID: 4

┌──────┬────────┬─────────┬─────────┐
│ Page │ Frame  │ Present │ Dirty   │
├──────┼────────┼─────────┼─────────┤
│  0   │   2    │   Yes   │   No    │
│  1   │   0    │   Yes   │   Yes   │
│  2   │   -    │   No    │   No    │
│  3   │   1    │   Yes   │   No    │
└──────┴────────┴─────────┴─────────┘
```

This will be useful during the viva.

---

# 32. Page Fault Event

Every page fault should generate an event.

Example:

```text
{
    "type": "PAGE_FAULT",
    "pid": 4,
    "page": 2,
    "reason": "PAGE_NOT_PRESENT"
}
```

The event can be displayed as:

```text
[Cycle 82]

PAGE FAULT

PID: 4
Page: 2
Reason: Page not present
```

---

# 33. Page Replacement Event

When page replacement occurs:

```text
{
    "type": "PAGE_REPLACEMENT",
    "pid": 4,
    "victim_page": 0,
    "new_page": 2,
    "frame": 1,
    "algorithm": "FIFO"
}
```

The dashboard can show:

```text
Page 0
   ↓
Removed from Frame 1
   ↓
Page 2
   ↓
Loaded into Frame 1
```

---

# 34. Memory Statistics

AIOS shall collect memory-related statistics.

Initial statistics:

```text
Total RAM
Used RAM
Free RAM
Total Frames
Used Frames
Page Fault Count
Page Replacement Count
Swap Usage
Pages Per Process
```

These statistics can be displayed on the dashboard.

---

# 35. Page Fault Statistics

For each process:

```text
PID
Page Faults
Pages Loaded
Pages Replaced
Swap Operations
```

Example:

```text
PID 4
Page Faults: 3
Pages Loaded: 3
Pages Replaced: 1
```

These values can also be used to compare memory-management scenarios.

---

# 36. Memory Error Handling

The Memory Manager shall distinguish between:

### Valid Page in RAM

```text
Valid = 1
Present = 1
```

Result:

```text
Normal Memory Access
```

### Valid Page Not in RAM

```text
Valid = 1
Present = 0
```

Result:

```text
Page Fault
```

### Invalid Page

```text
Valid = 0
```

Result:

```text
Invalid Memory Access
```

This distinction is important for demonstrating both virtual memory and error handling.

---

# 37. Memory and Interrupt Interaction

The Memory Manager does not directly perform the complete interrupt process.

Instead:

```text
Memory Manager
      │
      │ Page Not Present
      ▼
Interrupt Manager
      │
      ▼
Page Fault Handler
      │
      ▼
Memory Manager
```

This maintains separation of responsibilities.

---

# 38. Memory and Process Management

The Process Manager owns the process lifecycle.

The Memory Manager owns memory allocation and mapping.

```text
Process Manager
      │
      │ Allocate Memory
      ▼
Memory Manager
      │
      ▼
Page Table
      │
      ▼
PCB stores reference
```

When the process terminates:

```text
Process Termination
      │
      ▼
Memory Manager
      │
      ▼
Release Pages / Frames
```

---

# 39. Memory and Scheduler Interaction

The scheduler may use memory-related information as an input to the AI Adaptive Scheduler.

Example:

```text
Process P1
Memory Usage = Low

Process P2
Memory Usage = High
Page Faults = High
```

The AI scheduler may consider this information when calculating a scheduling score.

However, the normal scheduling algorithms do not need to depend on memory usage.

---

# 40. Memory and AI Agents

AI agents use the same virtual-memory system as normal processes.

Example:

```text
Research Agent
      │
      ▼
Process Representation
      │
      ▼
Page Table
      │
      ▼
Virtual Memory
```

The AI layer may track additional resource information, but it does not bypass the Memory Manager.

---

# 41. Example: AI Agent Page Fault

Suppose the Research Agent attempts to access Page 4.

```text
Research Agent
      │
      ▼
CPU Memory Access
      │
      ▼
Page Table
      │
      ▼
Page 4 = Not Present
      │
      ▼
PAGE FAULT
      │
      ▼
Interrupt Manager
      │
      ▼
Memory Manager
      │
      ▼
FIFO Page Replacement
      │
      ▼
Page 4 Loaded
      │
      ▼
Page Table Updated
      │
      ▼
Instruction Retried
      │
      ▼
Research Agent Continues
```

This is one of the main scenarios for the final demonstration.

---

# 42. Memory Manager Interfaces

The Memory Manager should expose functionality conceptually similar to:

```text
MemoryManager
│
├── allocateMemory()
├── releaseMemory()
├── translateAddress()
├── accessMemory()
├── handlePageFault()
├── allocateFrame()
├── releaseFrame()
├── replacePage()
├── loadPage()
├── swapOut()
├── swapIn()
├── getPageTable()
├── getMemoryState()
└── getMemoryStatistics()
```

The exact C++ method signatures will be finalized during implementation.

---

# 43. Memory Events

The Memory Manager shall generate events such as:

```text
PAGE_ALLOCATED
PAGE_ACCESSED
PAGE_FAULT
FRAME_ALLOCATED
PAGE_REPLACED
PAGE_LOADED
PAGE_SWAPPED_OUT
PAGE_SWAPPED_IN
INVALID_MEMORY_ACCESS
```

These events can be sent to the frontend.

---

# 44. Example Complete Memory Access

Consider:

```text
Page Size = 4
Logical Address = 10
```

Step 1:

```text
Page = 10 / 4 = 2
Offset = 10 % 4 = 2
```

Step 2:

```text
Page Table

Page 2 → Frame 1
Present = 1
```

Step 3:

```text
Physical Address =
(1 × 4) + 2
= 6
```

Therefore:

```text
CPU
 ↓
Logical Address 10
 ↓
Page 2 + Offset 2
 ↓
Page Table
 ↓
Frame 1
 ↓
Physical Address 6
 ↓
RAM
```

---

# 45. Example Complete Page Fault

Configuration:

```text
Page Size = 4
RAM Frames = 3
```

Current RAM:

```text
F0 → Page 1
F1 → Page 3
F2 → Page 5
```

Process requests:

```text
Page 2
```

No free frame exists.

Therefore:

```text
Page 2 Request
      ↓
No Free Frame
      ↓
FIFO
      ↓
Select Page 1
      ↓
Remove Page 1
      ↓
Load Page 2 into F0
      ↓
Update Page Table
      ↓
Resume Process
```

Final RAM:

```text
F0 → Page 2
F1 → Page 3
F2 → Page 5
```

---

# 46. Success Criteria

The Paging and Virtual Memory subsystem is complete when:

* Logical memory can be divided into pages.
* Physical RAM can be divided into frames.
* Page tables can be created.
* Pages can be mapped to frames.
* Logical addresses can be translated.
* Present/valid state can be tracked.
* Page faults can be detected.
* Page faults can generate interrupts.
* Required pages can be loaded.
* Page replacement works.
* FIFO replacement works.
* Swap space is simulated.
* Virtual memory can exceed physical RAM.
* Invalid memory accesses are detected.
* Memory is released when a process terminates.
* Memory statistics are collected.
* Memory events are visible in the dashboard.

---

# 47. Summary

The AIOS Memory Manager provides the virtual memory system used by every process and AI agent.

The core flow is:

```text
             Logical Memory
                    │
                    ▼
                  Pages
                    │
                    ▼
               Page Table
                    │
          ┌─────────┴─────────┐
          │                   │
      Present             Not Present
          │                   │
          ▼                   ▼
       Frame              Page Fault
          │                   │
          ▼                   ▼
        RAM              Interrupt
                              │
                              ▼
                       Page Replacement
                              │
                              ▼
                         Load Page
                              │
                              ▼
                         Update Table
                              │
                              ▼
                       Resume Process
```

Virtual memory allows AIOS to simulate a process using a logical memory space larger than physical RAM, while paging provides the mechanism for mapping that logical memory onto physical frames.

The Memory Manager therefore connects:

```text
CPU
 │
Process Manager
 │
Interrupt Manager
 │
Swap
 │
Physical RAM
```

and provides the foundation required for the Stage II and Stage III memory-management demonstrations.
