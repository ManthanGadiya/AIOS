// API Layer - REST communication with C++ backend
// Per docs/13_GUI_Architecture.md sections 51-53, 56

import type { 
  SimulationControl, SystemStatistics, Process, Agent, CPUState, MemoryFrame, PageTable, 
  SchedulerState, InterruptEvent, EventLogEntry, Mutex, Semaphore, IpcSendRequest, 
  IpcReceiveRequest, IpcSendResponse, IpcReceiveResponse, SyncLockRequest, SyncUnlockRequest, 
  SyncWaitRequest, SyncSignalRequest, SyncLockResponse, SyncUnlockResponse, SyncWaitResponse, 
  SyncSignalResponse, ApiResponse, WSMessage, AgentStatistics, Checkpoint 
} from '../types/os';

// Type exports
export type { SimulationControl } from '../types/os';

const API_BASE = import.meta.env.VITE_API_BASE || 'http://localhost:8081/api';
const WS_BASE = import.meta.env.VITE_WS_BASE || 'ws://localhost:8081/ws';

class ApiService {
  private async request<T>(endpoint: string, options: RequestInit = {}): Promise<ApiResponse<T>> {
    try {
      const response = await fetch(`${API_BASE}${endpoint}`, {
        headers: {
          'Content-Type': 'application/json',
          ...options.headers,
        },
        ...options,
      });
      const data = await response.json();
      return { success: response.ok, data: response.ok ? data : undefined, error: response.ok ? undefined : data?.error || 'Request failed' };
    } catch (error) {
      return { success: false, error: error instanceof Error ? error.message : 'Network error' };
    }
  }

  // Simulation Controls
  sendCommand(command: SimulationControl) {
    return this.request<{ success: boolean }>('/command', {
      method: 'POST',
      body: JSON.stringify(command),
    });
  }

  startSimulation() {
    return this.sendCommand({ command: 'START_SIMULATION' });
  }

  pauseSimulation() {
    return this.sendCommand({ command: 'PAUSE_SIMULATION' });
  }

  stopSimulation() {
    return this.sendCommand({ command: 'STOP_SIMULATION' });
  }

  resetSimulation() {
    return this.sendCommand({ command: 'RESET_SIMULATION' });
  }

  changeScheduler(policy: string) {
    return this.sendCommand({ command: 'CHANGE_SCHEDULER', payload: { policy } });
  }

  runScenario(scenario: string) {
    return this.sendCommand({ command: 'RUN_SCENARIO', payload: { scenario } });
  }

  // State Queries
  getStatistics() {
    return this.request<SystemStatistics>('/statistics');
  }

  getProcesses() {
    return this.request<Process[]>('/processes');
  }

  getAllProcesses() {
    return this.request<any[]>('/processes');
  }

  getAgents() {
    return this.request<Agent[]>('/agents');
  }

  getCPU() {
    return this.request<CPUState>('/cpu');
  }

  getMemory() {
    return this.request<{ frames: MemoryFrame[]; pageTables: PageTable[] }>('/memory');
  }

  getPageTables() {
    return this.request<PageTable[]>('/memory/page-tables');
  }

  getScheduler() {
    return this.request<SchedulerState>('/scheduler');
  }

  getInterrupts() {
    return this.request<InterruptEvent[]>('/interrupts');
  }

  getEvents() {
    return this.request<EventLogEntry[]>('/events');
  }

  getIPC() {
    return this.request<any>('/ipc');
  }

  getIO() {
    return this.request<any>('/io');
  }

  getSync() {
    return this.request<any>('/sync');
  }

  getMutexes() {
    return this.request<Mutex[]>('/mutexes');
  }

  getSemaphores() {
    return this.request<Semaphore[]>('/semaphores');
  }

  // Configuration
  setClockSpeed(speed: number) {
    return this.sendCommand({ command: 'SET_CLOCK_SPEED', payload: { speed } });
  }

  setQuantum(quantum: number) {
    return this.sendCommand({ command: 'SET_QUANTUM', payload: { quantum } });
  }

  reset() {
    return this.sendCommand({ command: 'RESET_SIMULATION' });
  }

  // IPC Operations
  sendIpcMessage(request: IpcSendRequest) {
    return this.request<IpcSendResponse>('/ipc/send', {
      method: 'POST',
      body: JSON.stringify(request),
    });
  }

  receiveIpcMessage(request: IpcReceiveRequest) {
    return this.request<IpcReceiveResponse>('/ipc/receive', {
      method: 'POST',
      body: JSON.stringify(request),
    });
  }

  // Sync Operations
  lockMutex(request: SyncLockRequest) {
    return this.request<SyncLockResponse>('/sync/lock', {
      method: 'POST',
      body: JSON.stringify(request),
    });
  }

  unlockMutex(request: SyncUnlockRequest) {
    return this.request<SyncUnlockResponse>('/sync/unlock', {
      method: 'POST',
      body: JSON.stringify(request),
    });
  }

  waitSemaphore(request: SyncWaitRequest) {
    return this.request<SyncWaitResponse>('/sync/wait', {
      method: 'POST',
      body: JSON.stringify(request),
    });
  }

  signalSemaphore(request: SyncSignalRequest) {
    return this.request<SyncSignalResponse>('/sync/signal', {
      method: 'POST',
      body: JSON.stringify(request),
    });
  }

  // AI Agent Operations
  getAgentDetails() {
    return this.request<AgentStatistics[]>('/agents/detail');
  }

  createAgent(name: string, type: 'RESEARCH' | 'CODING', priority: number, tokenBudget: number) {
    return this.request<{ success: boolean; pid: number; error?: string }>('/agent/create', {
      method: 'POST',
      body: JSON.stringify({ name, type, priority, tokenBudget }),
    });
  }

  createCheckpoint(agentId: string, progress: string) {
    return this.request<{ success: boolean; checkpointId: string; error?: string }>('/agent/checkpoint', {
      method: 'POST',
      body: JSON.stringify({ agentId, progress }),
    });
  }

  recoverAgent(agentId: string, checkpointId?: string) {
    return this.request<{ success: boolean; error?: string }>('/agent/recover', {
      method: 'POST',
      body: JSON.stringify({ agentId, checkpointId: checkpointId || '' }),
    });
  }

  failAgent(agentId: string, failureType: string = 'SIMULATED_CRASH') {
    return this.request<{ success: boolean; error?: string }>('/agent/fail', {
      method: 'POST',
      body: JSON.stringify({ agentId, failureType }),
    });
  }

  requestToolPermission(agentId: string, tool: string) {
    return this.request<{ success: boolean; allowed: boolean; tool: string }>('/agent/tool-request', {
      method: 'POST',
      body: JSON.stringify({ agentId, tool }),
    });
  }

  getAgentCheckpoints(agentId: string) {
    return this.request<Checkpoint[]>(`/agent/checkpoints?agentId=${encodeURIComponent(agentId)}`);
  }

  getAgentQuota(agentId: string) {
    return this.request<any>(`/agent/quota?agentId=${encodeURIComponent(agentId)}`);
  }
}

export const api = new ApiService();

// WebSocket Layer - Live simulation events
// Per docs/13_GUI_Architecture.md sections 51-54

type WSHandler = (message: WSMessage) => void;

class WebSocketService {
  private ws: WebSocket | null = null;
  private handlers: Map<string, WSHandler[]> = new Map();
  private reconnectAttempts = 0;
  private maxReconnectAttempts = 5;
  private reconnectDelay = 1000;

  connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      try {
        this.ws = new WebSocket(WS_BASE);

        this.ws.onopen = () => {
          console.log('[WS] Connected to backend');
          this.reconnectAttempts = 0;
          resolve();
        };

        this.ws.onmessage = (event) => {
          try {
            const message: WSMessage = JSON.parse(event.data);
            this.dispatch(message);
          } catch (error) {
            console.error('[WS] Failed to parse message:', error);
          }
        };

        this.ws.onclose = () => {
          console.log('[WS] Disconnected from backend');
          this.attemptReconnect();
        };

        this.ws.onerror = (error) => {
          console.error('[WS] Error:', error);
          reject(error);
        };
      } catch (error) {
        reject(error);
      }
    });
  }

  private attemptReconnect() {
    if (this.reconnectAttempts < this.maxReconnectAttempts) {
      this.reconnectAttempts++;
      const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1);
      console.log(`[WS] Reconnecting in ${delay}ms (attempt ${this.reconnectAttempts})`);
      setTimeout(() => this.connect(), delay);
    } else {
      console.error('[WS] Max reconnect attempts reached');
    }
  }

  private dispatch(message: WSMessage) {
    const handlers = this.handlers.get(message.type) || [];
    handlers.forEach(handler => handler(message));
    
    // Also call wildcard handlers
    const wildcardHandlers = this.handlers.get('*') || [];
    wildcardHandlers.forEach(handler => handler(message));
  }

  subscribe(type: string, handler: WSHandler) {
    if (!this.handlers.has(type)) {
      this.handlers.set(type, []);
    }
    this.handlers.get(type)!.push(handler);
    
    // Return unsubscribe function
    return () => {
      const handlers = this.handlers.get(type) || [];
      const index = handlers.indexOf(handler);
      if (index > -1) handlers.splice(index, 1);
    };
  }

  unsubscribe(type: string, handler: WSHandler) {
    const handlers = this.handlers.get(type) || [];
    const index = handlers.indexOf(handler);
    if (index > -1) handlers.splice(index, 1);
  }

  send(message: WSMessage) {
    if (this.ws?.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(message));
    }
  }

  disconnect() {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  }

  get isConnected() {
    return this.ws?.readyState === WebSocket.OPEN;
  }
}

export const ws = new WebSocketService();

// Event type constants matching backend
export const WS_EVENT_TYPES = {
  PROCESS_STATE_CHANGED: 'PROCESS_STATE_CHANGED',
  CONTEXT_SWITCH: 'CONTEXT_SWITCH',
  PAGE_FAULT: 'PAGE_FAULT',
  PAGE_LOADED: 'PAGE_LOADED',
  INTERRUPT_GENERATED: 'INTERRUPT_GENERATED',
  INTERRUPT_HANDLED: 'INTERRUPT_HANDLED',
  IPC_SEND: 'IPC_SEND',
  IPC_RECEIVE: 'IPC_RECEIVE',
  IO_REQUEST: 'IO_REQUEST',
  IO_COMPLETE: 'IO_COMPLETE',
  MUTEX_LOCK: 'MUTEX_LOCK',
  MUTEX_UNLOCK: 'MUTEX_UNLOCK',
  AGENT_CREATED: 'AGENT_CREATED',
  AGENT_FAILURE: 'AGENT_FAILURE',
  CHECKPOINT_CREATED: 'CHECKPOINT_CREATED',
  AGENT_RECOVERED: 'AGENT_RECOVERED',
  SCHEDULER_DECISION: 'SCHEDULER_DECISION',
  CPU_UPDATE: 'CPU_UPDATE',
  STATISTICS_UPDATE: 'STATISTICS_UPDATE',
} as const;