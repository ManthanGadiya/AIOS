// AIOS Frontend TypeScript Types
// Matching backend C++ data structures per docs/13_GUI_Architecture.md section 56

export type ProcessState = 'NEW' | 'READY' | 'RUNNING' | 'WAITING' | 'TERMINATED' | 'FAILED';
export type ProcessType = 'NORMAL' | 'AI_AGENT';
export type SchedulingPolicy = 'FCFS' | 'ROUND_ROBIN' | 'PRIORITY' | 'AI_ADAPTIVE';
export type AgentState = 'ACTIVE' | 'WAITING' | 'CHECKPOINTED' | 'FAILED' | 'RECOVERED';
export type AgentType = 'RESEARCH' | 'CODING';
export type ToolPermission = 'READ_DATA' | 'SEARCH' | 'WRITE_CODE' | 'EXECUTE';
export type FailureType = 'RESOURCE_LIMIT' | 'SIMULATED_CRASH' | 'EXECUTION_ERROR' | 'TOOL_PERMISSION_ERROR' | 'MEMORY_ERROR';
export type InterruptType = 'TIMER' | 'PAGE_FAULT' | 'SYSTEM_CALL' | 'IO_COMPLETE' | 'IPC_EVENT' | 'ERROR';
export type InterruptStatus = 'PENDING' | 'SERVICING' | 'COMPLETED';
export type DeviceState = 'IDLE' | 'BUSY';
export type MutexState = 'LOCKED' | 'FREE';
export type SemaphoreState = 'AVAILABLE' | 'WAITING';

export interface CPUState {
  pc: number;
  ir: number;
  acc: number;
  mar: number;
  mbr: number;
  flags: {
    zero: boolean;
    negative: boolean;
    carry: boolean;
    overflow: boolean;
    error: boolean;
  };
}

export interface Process {
  pid: number;
  type: ProcessType;
  name: string;
  state: ProcessState;
  priority: number;
  memory: number; // MB or pages
  cpuTime: number; // cycles
  readySinceCycle?: number;
  waitingTime?: number;
  responseTime?: number;
  turnaroundTime?: number;
  contextSwitchCount?: number;
  pageFaultCount?: number;
  // AI Agent specific fields (when type === 'AI_AGENT')
  agentId?: string;
  task?: string;
  tokenUsage?: { used: number; budget: number };
  permissions?: Record<string, boolean>;
}

export interface Agent extends Process {
  agentId: string;
  task: string;
  tokenUsage: { used: number; budget: number };
  permissions: Record<string, boolean>;
  agentState?: AgentState;
}

export interface MemoryFrame {
  frameId: number;
  owner: string; // PID or 'Free'
  processId?: number;
  pageId?: number;
}

export interface PageTableEntry {
  page: number;
  frame: number | null;
  valid: boolean;
  modified: boolean;
  referenced: boolean;
}

export interface PageTable {
  processId: number;
  entries: PageTableEntry[];
}

export interface InterruptEvent {
  id: number;
  timestamp: string;
  type: InterruptType;
  description: string;
  status: InterruptStatus;
  pid?: number;
}

export interface IpcMessage {
  id: number;
  senderPid: number;
  receiverPid: number;
  type: string;
  payload: string;
  timestamp: number;
  channelId?: string;
}

export interface IODevice {
  name: string;
  state: DeviceState;
  queue: string[]; // Process names waiting
}

export interface Mutex {
  id: string;
  locked: boolean;
  ownerPid: number;
  waiters: number[];
  waiterCount: number;
}

export interface Semaphore {
  id: string;
  count: number;
  waiters: number[];
  waiterCount: number;
}

export interface IpcMessage {
  id: number;
  senderPid: number;
  receiverPid: number;
  type: string;
  payload: string;
  timestamp: number;
  channelId?: string;
}

export interface IpcSendRequest {
  senderPid: number;
  receiverPid: number;
  type: string;
  payload: string;
}

export interface IpcReceiveRequest {
  receiverPid: number;
  blocking?: boolean;
}

export interface IpcSendResponse {
  messageId: number;
  success: boolean;
  error?: string;
}

export interface IpcReceiveResponse {
  success: boolean;
  blocking: boolean;
  message?: IpcMessage;
}

export interface SyncLockRequest {
  mutexId: string;
  pid: number;
}

export interface SyncUnlockRequest {
  mutexId: string;
  pid: number;
}

export interface SyncWaitRequest {
  semaphoreId: string;
  pid: number;
}

export interface SyncSignalRequest {
  semaphoreId: string;
}

export interface SyncLockResponse {
  success: boolean;
}

export interface SyncUnlockResponse {
  success: boolean;
  nextWaiter: number;
}

export interface SyncWaitResponse {
  success: boolean;
}

export interface SyncSignalResponse {
  success: boolean;
  nextWaiter: number;
}

export interface SchedulerState {
  policy: SchedulingPolicy;
  currentAlgorithm: SchedulingPolicy;
  currentProcess: number | null;
  readyQueue: number[]; // PIDs in order
  timeQuantum: number;
  schedulingScore?: number;
  currentTimeCycle?: number;
  contextSwitchCount?: number;
  decisionCount?: number;
  schedulingHistory: SchedulingDecision[];
  metrics: SchedulerMetrics;
}

export interface SchedulingDecision {
  cycle: number;
  policy: SchedulingPolicy;
  selectedPid: number;
  preemptedPid: number | null;
  reason: string;
}

export interface SchedulerMetrics {
  avgWaitingTime: number;
  avgTurnaroundTime: number;
  avgResponseTime: number;
  cpuUtilization: number;
  contextSwitchCount: number;
  throughput: number;
}

export interface SystemStatistics {
  totalProcesses: number;
  aiAgents: number;
  cpuUsage: number; // percentage
  memoryUsage: number; // percentage
  pageFaults: number;
  interrupts: number;
  cycle?: number;
}

export interface EventLogEntry {
  id: number;
  timestamp: string;
  category: 'PROCESS' | 'CPU' | 'MEMORY' | 'SCHEDULER' | 'INTERRUPT' | 'IPC' | 'IO' | 'SYNCHRONIZATION' | 'AI_AGENT' | 'ERROR';
  message: string;
  pid?: number;
}

// Alias for EventLogEntry for use in Logs page
export type Event = EventLogEntry;

// Process statistics type for Processes page
export interface ProcessStatistics {
  pid: number;
  name: string;
  type: ProcessType;
  state: ProcessState;
  priority: number;
  baseAddress: number;
  programSize: number;
  arrivalTime: number;
  cpuTime: number;
  waitingTime: number;
  turnaroundTime: number;
  responseTime: number;
  stateChanges: number;
  contextSwitches: number;
  pageFaults: number;
  ioRequests: number;
  ipcOperations: number;
}

// Agent statistics type for AI Agents page
export interface AgentStatistics {
  pid: number;
  agentId: string;
  name: string;
  type: ProcessType;
  state: ProcessState;
  agentState?: AgentState;
  priority: number;
  task: string;
  tokenUsage: { used: number; budget: number };
  permissions: Record<string, boolean>;
  cpuTime: number;
  memoryUsage: number;
  ipcSent: number;
  ipcReceived: number;
  checkpointCount: number;
  failureCount: number;
  recoveryCount: number;
  lastCheckpointId?: string;
}

// Checkpoint type for AI Agent checkpoints
export interface Checkpoint {
  checkpointId: string;
  createdCycle: number;
  agentId: string;
  pid: number;
  progress: string;
  cpuContext: {
    pc: number;
    acc: number;
    mar: number;
    mbr: number;
    ir: number;
    flags: {
      zero: boolean;
      negative: boolean;
      carry: boolean;
      overflow: boolean;
      error: boolean;
    };
  };
  processState: ProcessState;
  pageCount: number;
}

// I/O state type
export interface IOState {
  devices: IODevice[];
  buffers: Array<{
    name: string;
    capacity: number;
    used: number;
    state: 'EMPTY' | 'PARTIAL' | 'FULL';
  }>;
  spoolers: Array<{
    deviceName: string;
    queue: string[];
    active: boolean;
  }>;
}

// IPC state type
export interface IPCState {
  channels: Array<{
    pid: number;
    messageCount: number;
    capacity: number;
  }>;
  messages: Array<{
    id: number;
    senderPid: number;
    receiverPid: number;
    type: string;
    payload: string;
    timestamp: number;
  }>;
  permissions: Array<{
    senderPid: number;
    receiverPid: number;
    allowed: boolean;
  }>;
  statistics: {
    messagesSent: number;
    messagesReceived: number;
    failedSends: number;
    permissionDenials: number;
  };
}

// Interrupt state type
export interface InterruptState {
  pending: InterruptEvent[];
  completed: InterruptEvent[];
  currentPhase: 'IDLE' | 'PENDING' | 'SERVICING' | 'COMPLETED';
}

// Memory state type
export interface MemoryState {
  totalWords: number;
  usedWords: number;
  pageSize: number;
  totalFrames: number;
  usedFrames: number;
  pageFaultCount: number;
  pageReplacementCount: number;
  swapWords: number;
  frames: MemoryFrame[];
}

export interface SimulationControl {
  command: 'START_SIMULATION' | 'PAUSE_SIMULATION' | 'STOP_SIMULATION' | 'RESET_SIMULATION' | 'CHANGE_SCHEDULER' | 'RUN_SCENARIO' | 'GENERATE_PAGE_FAULT' | 'SIMULATE_AGENT_FAILURE' | 'SET_CLOCK_SPEED' | 'SET_QUANTUM' | 'CREATE_AGENT' | 'CHECKPOINT_AGENT' | 'RECOVER_AGENT' | 'FAIL_AGENT' | 'REQUEST_TOOL';
  payload?: Record<string, unknown>;
}

export interface WSMessage {
  type: string;
  payload: unknown;
}

export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  error?: string;
}