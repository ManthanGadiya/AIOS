# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Project definition and documentation baseline:
  - PRD and subsystem specifications (docs/01 … docs/15)
  - ROADMAP with phased implementation plan
- Governance and collaboration documents:
  - AGENTS.md — development-agent workflow and project rules
  - TEAMMATES.md — team and collaboration tracker
  - STATUS.md — living project status
  - CONTRIBUTING.md — contribution guidelines
  - CHANGELOG.md — this file
- Canonical repository structure:
  - backend/ — C++ simulation engine (source of truth)
  - frontend/ — React dashboard
  - tests/ — unit and integration tests
- MIT license (Copyright 2026 Manthan Gadiya)

### Fixed

- Canonical documentation filenames confirmed as plural (`AGENTS.md`, `14_Demo_Scenarios.md`); stale references removed.
- Repository structure trees in docs/PRD.md and docs/ROADMAP.md aligned with the actual repository.

## [2026-10-28] — Week 7: I/O Subsystem + Syscall Implementation

### Added

- **I/O Subsystem** (docs/11):
  - `Device` class — DISK/PRINTER/KEYBOARD/NETWORK with processing time, FIFO request queue, state machine (AVAILABLE/BUSY/ERROR), tick-based processing
  - `Buffer` class — EMPTY/PARTIAL/FULL states, capacity-limited write/read, statistics tracking
  - `Spooler` class — job queue per device, concurrent I/O processing, spool job lifecycle
  - `IOManager` — device/buffer/spooler management, `requestIO()`, `tick()` for device processing, process cleanup, statistics (total/completed/failed requests, interrupts, buffer events, spool jobs)
  - New event types: IO_REQUEST, IO_COMPLETE, IO_STARTED, IO_BLOCKED, IO_ERROR

- **System Call Implementation** — `SYSCALL_IO_REQUEST=6` moved from stub to full implementation:
  - `SyscallRequest` extended with `payload` string field for complex syscalls
  - `SystemCallManager` payload storage (`setSyscallPayload()`) with per-PID consumption
  - `handleIoRequest()` parses `"deviceId:operation:data"` payload (operation: 0=READ, 1=WRITE)
  - Calls `IOManager::requestIO()` and blocks calling process via `ProcessManager::blockProcess()`
  - On completion, `IOManager::tick()` generates `IO_COMPLETE` interrupt → `ProcessManager::wakeProcess()`
  - API endpoint: `POST /api/command` with `SYSCALL` command accepting `{pid, syscallNumber, payload}`

### Changed

- `SystemCallManager` event logging now uses 4-arg `EventLog::record(type, pid, cycle, detail)` with clock cycle
- Synchronization syscalls (LOCK/UNLOCK/WAIT/SIGNAL) fixed to match `SynchronizationManager` signatures
- `EventLog` calls updated to include cycle for all syscalls

## [2026-10-27] — Week 6: Synchronization + IPC

### Added

- **Synchronization Subsystem** (docs/09):
  - `Mutex` class — ownership tracking, FIFO wait queue, tryLock/unlock/forceRelease
  - `Semaphore` class — counting semaphore, wait/signal, FIFO wait queue
  - `SynchronizationManager` — createMutex/lockMutex/unlockMutex, createSemaphore/waitSemaphore/signalSemaphore, cleanupProcess (force-release on termination), deadlock detection via wait-for graph DFS, list/get for dashboard
  - Deadlock detection — wait-for graph with DFS cycle detection; detectDeadlock() and getDeadlockCycle()
  - Race condition demonstration — T-SYNC-016/017 show counter corruption without mutex vs correct result with mutex
  - New event types: MUTEX_CREATED/LOCKED/UNLOCKED, PROCESS_BLOCKED_ON_MUTEX/WAKEUP, SEMAPHORE_CREATED/WAIT/SIGNAL, DEADLOCK_DETECTED, SYNCHRONIZATION_ERROR
  - New system calls: SYSCALL_LOCK=7, UNLOCK=8, WAIT=9, SIGNAL=10 (stubs)
  - Unit tests: test_sync.cpp (21 T-SYNC cases)

- **IPC Subsystem** (docs/10):
  - `IPCMessage` struct — id, senderPid, receiverPid, type, payload, timestamp
  - `MessageQueue` class — FIFO, capacity limit (10), enqueue/dequeue/peek, blocking/non-blocking receive, removeMessagesFor
  - `IPCManager` class — createChannel/sendMessage/receiveMessage/hasMessage, grantPermission/revokePermission/checkPermission (allow/deny lists), getStatistics/getProcessStatistics, cleanupProcess, reset
  - Permission model — allow-list / deny-list with default-allow; explicit grant/revoke per sender→receiver
  - Statistics — messagesSent/received/pending, failedSends, permissionDenials; per-process statistics
  - Agent communication patterns — research→coding result delivery, task request/response
  - Process cleanup — removes messages addressed TO terminated process; cleans permissions and channel
  - New event types: IPC_CHANNEL_CREATED, IPC_SEND/RECEIVE, IPC_MESSAGE_QUEUED/DELIVERED, IPC_QUEUE_FULL, IPC_PERMISSION_DENIED, IPC_ERROR
  - New system call: SYSCALL_IPC_RECEIVE=11 (stub)
  - Unit tests: test_ipc.cpp (22 T-IPC cases)

### Changed

- Permission system refactored from single allow-list to dual allow/deny lists for proper revoke semantics
- cleanupProcess now only removes messages addressed TO the terminated process (receiverPid), preserving messages sent BY the process

### Fixed

- Revoke permission now works even without prior grant (creates deny entry)
- Default permission model: allow all when no explicit allow/deny configured (standard OS IPC behavior)

## [2026-08-22] — Mid-Review GUI: REST Server + React Dashboard

### Added

- C++ REST API server (`aios/server`, cpp-httplib + nlohmann/json) exposing the engine: /health, /api/statistics, /api/processes, /api/agents, /api/cpu, /api/memory, /api/scheduler, /api/interrupts, /api/events, POST /api/command
- Simulation control: START/PAUSE/STOP gating 10 cycles/s host loop; RESET restores initial workload; CHANGE_SCHEDULER maps to SchedulingPolicy
- Demo workload (PRD section 20): P1 TextEditor, P2 Compiler, P3 MusicPlayer + A1 Research Agent, A2 Coding Agent (createAgentProcess)
- React 19 + TypeScript + Tailwind CSS v4 (Vite) dashboard: control bar, sidebar, stat cards, process table, CPU panel, frame grid, page tables, ready queue, event log, interrupt panel
- Build layout consolidated: canonical build directory is backend/build
- Port-conflict guard: server probes /health before binding
- Real dashboard statistics from ProcessManager/MemoryManager/Scheduler APIs
- Connection honesty: Engine Connected/Disconnected badge, disconnection banner
- Scheduler panel with Gantt timeline from scheduling history

## [2026-08-20] — M5-Scheduler

### Added

- SchedulingPolicy (FCFS / ROUND_ROBIN / PRIORITY) with policy name accessor
- Persistent ready order reconciled with Process Manager's READY set
- FCFS (non-preemptive FIFO), Round Robin (quantum expiry preempts), Priority (preemptive, tie-breaks by ready time then PID)
- Scheduler events: SCHEDULER_STARTED, PROCESS_SELECTED, PROCESS_PREEMPTED, TIME_QUANTUM_EXPIRED
- Decision history, context-switch count, reset()
- TIMER interrupt drives Round Robin via InterruptManager::setScheduler()
- Unit tests: test_scheduler.cpp (16 T-SCHED cases)

## [2026-08-20] — M5: Process Manager Full Features

### Added

- ProcessType (NORMAL / AI_AGENT) on PCB + createAgentProcess() wrapper
- CPU-time accounting (charged on CPU release), waiting-time accounting (charged leaving READY)
- Response time (first run), turnaround time, state-change and context-switch counters
- Statistics API: getProcessStatistics(), getAllProcessStatistics(), cpuUtilization()
- Per-process page faults from Memory Manager
- Unit tests: T-PROC-014..023 + T-PAGE-015

## [2026-08-20] — M4: Full Interrupt Manager

### Added

- Interrupt priority queue: ERROR > PAGE_FAULT > SYSTEM_CALL > IO_COMPLETE > TIMER
- Nested-interrupt policy: deferred, never nested
- Timer component: periodic TIMER interrupts (quantum 4 cycles)
- Lifecycle observability (SERVICING → COMPLETED) and pendingInterrupts() accessor
- docs/05 section 19 updated with finalized priority ordering
- Unit tests: test_timer.cpp (7 T-TIMER) + T-INT-009/010/011

## [2026-08-20] — M3: Paging / Virtual Memory

### Added

- Memory Manager: page tables, frame table, simulated swap, demand paging, FIFO page replacement with dirty write-back
- Address translation with INVALID vs PAGE_FAULT distinction; MemoryAccessResult API
- PAGE_FAULT interrupt type; Interrupt Manager dispatches faults
- CPU integration: paged fetch/execute with instruction retry; invalid access fails via ERROR interrupt
- Process Manager integration: paged createProcess (logical base 0) with legacy flat-Memory fallback
- Memory statistics and 8 new memory event types
- Unit tests: 14 T-PAGE cases

## [2026-08-19] — M1+M2: Core OS Implementation

### Added

- CPU Simulator: registers, fetch-decode-execute, full instruction set, FLAGS, error detection
- Memory: 1024-word flat RAM, program loading, bounds checking
- Process Manager: PCB, state machine, queues, dispatch/preemption with context save/restore
- Interrupt Manager: SYSTEM_CALL, TIMER, IO_COMPLETE, ERROR, pending queue + log
- System Call Manager: READ, WRITE, ALLOCATE, EXIT (stubs for IPC, I/O)
- Event Log, Simulation Clock, Program Loader
- All unit tests passing (9 test files)
- CMake + MinGW build verified
- GitHub repository confirmed and synced

## [2026-08-18] — Governance Documents

### Added

- STATUS.md and TEAMMATES.md populated from AGENTS.md copies
- Canonical documentation filenames confirmed as plural
- Filename references corrected across README.md, AGENTS.md, docs/PRD.md, docs/01_Project_Overview.md
- Canonical repository structure established: backend/, frontend/, tests/
