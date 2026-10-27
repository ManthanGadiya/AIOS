// AIOS Frontend TypeScript Types
// Matching backend C++ data structures per docs/13_GUI_Architecture.md section 56

export type ProcessState = 'NEW' | 'READY' | 'RUNNING' | 'WAITING' | 'TERMINATED' | 'FAILED';
export type ProcessType = 'NORMAL' | 'AI_AGENT';
export type SchedulingPolicy = 'FCFS' | 'ROUND_ROBIN' | 'PRIORITY' | 'AI_ADAPTIVE';
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

export interface SimulationControl {
  command: 'START_SIMULATION' | 'PAUSE_SIMULATION' | 'STOP_SIMULATION' | 'RESET_SIMULATION' | 'CHANGE_SCHEDULER' | 'RUN_SCENARIO' | 'GENERATE_PAGE_FAULT' | 'SIMULATE_AGENT_FAILURE';
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