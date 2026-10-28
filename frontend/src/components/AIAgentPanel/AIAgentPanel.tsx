import { useEffect, useState } from 'react';
import { api } from '../../services/api';
import type { Agent, AgentStatistics, Checkpoint } from '../../types/os';

interface AIAgentPanelProps {
  autoRefresh?: boolean;
  refreshInterval?: number;
}

export function AIAgentPanel({ autoRefresh = true, refreshInterval = 3000 }: AIAgentPanelProps) {
  const [agents, setAgents] = useState<Agent[]>([]);
  const [agentDetails, setAgentDetails] = useState<AgentStatistics[]>([]);
  const [agentCheckpoints, setAgentCheckpoints] = useState<Checkpoint[]>([]);
  const [showCheckpoints, setShowCheckpoints] = useState<string | null>(null);
  const [actionLoading, setActionLoading] = useState<string | null>(null);

  const fetchAgents = async () => {
    try {
      const agentsRes = await api.getAgents();
      if (agentsRes.success && agentsRes.data) setAgents(agentsRes.data);

      const detailsRes = await api.getAgentDetails();
      if (detailsRes.success && detailsRes.data) setAgentDetails(detailsRes.data);
    } catch (error) {
      console.error('Failed to fetch agents:', error);
    }
  };

  const handleCreateCheckpoint = async (agentId: string) => {
    setActionLoading(agentId);
    try {
      const res = await api.createCheckpoint(agentId, 'Manual checkpoint');
      if (res.success) {
        fetchAgents();
      }
    } catch (e) {
      console.error('Failed to create checkpoint:', e);
    } finally {
      setActionLoading(null);
    }
  };

  const handleRecoverAgent = async (agentId: string, checkpointId?: string) => {
    setActionLoading(agentId);
    try {
      const res = await api.recoverAgent(agentId, checkpointId);
      if (res.success) {
        fetchAgents();
        setShowCheckpoints(null);
      }
    } catch (e) {
      console.error('Failed to recover agent:', e);
    } finally {
      setActionLoading(null);
    }
  };

  const handleFailAgent = async (agentId: string) => {
    if (!window.confirm('Simulate agent failure? This will crash the agent.')) return;
    setActionLoading(agentId);
    try {
      const res = await api.failAgent(agentId, 'SIMULATED_CRASH');
      if (res.success) {
        fetchAgents();
      }
    } catch (e) {
      console.error('Failed to fail agent:', e);
    } finally {
      setActionLoading(null);
    }
  };

  const handleRequestTool = async (agentId: string, tool: string) => {
    setActionLoading(`${agentId}-${tool}`);
    try {
      const res = await api.requestToolPermission(agentId, tool);
      if (res.success) {
        fetchAgents();
      }
    } catch (e) {
      console.error('Failed to request tool:', e);
    } finally {
      setActionLoading(null);
    }
  };

  const handleViewCheckpoints = async (agentId: string) => {
    if (showCheckpoints === agentId) {
      setShowCheckpoints(null);
      return;
    }
    try {
      const res = await api.getAgentCheckpoints(agentId);
      if (res.success && res.data) {
        setAgentCheckpoints(res.data);
        setShowCheckpoints(agentId);
      }
    } catch (e) {
      console.error('Failed to fetch checkpoints:', e);
    }
  };

  useEffect(() => {
    fetchAgents();
    if (autoRefresh) {
      const interval = setInterval(fetchAgents, refreshInterval);
      return () => clearInterval(interval);
    }
  }, [autoRefresh, refreshInterval]);

  const getTokenPercentage = (tokenUsage: { used: number; budget: number }) => {
    return tokenUsage.budget > 0 ? Math.round((tokenUsage.used / tokenUsage.budget) * 100) : 0;
  };

  const getStateColor = (state: string) => {
    switch (state) {
      case 'RUNNING': return 'bg-green-100 text-green-800';
      case 'READY': return 'bg-blue-100 text-blue-800';
      case 'WAITING': return 'bg-yellow-100 text-yellow-800';
      case 'FAILED': return 'bg-red-100 text-red-800';
      case 'TERMINATED': return 'bg-gray-100 text-gray-800';
      default: return 'bg-gray-100 text-gray-800';
    }
  };

  const getAgentStateColor = (state: string | undefined) => {
    switch (state) {
      case 'ACTIVE': return 'bg-green-100 text-green-800';
      case 'WAITING': return 'bg-yellow-100 text-yellow-800';
      case 'CHECKPOINTED': return 'bg-purple-100 text-purple-800';
      case 'FAILED': return 'bg-red-100 text-red-800';
      case 'RECOVERED': return 'bg-blue-100 text-blue-800';
      default: return 'bg-gray-100 text-gray-800';
    }
  };

  return (
    <div className="bg-gray-800 rounded-lg p-4 h-full flex flex-col">
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-lg font-semibold text-white flex items-center gap-2">
          <svg className="w-5 h-5 text-purple-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9.663 17h4.673M12 3v1m6.364 1.636l-.707.707M21 12h-1M4 12H3m3.343-5.657l-.707-.707m2.828 9.9a5 5 0 117.072 0l-.548.547A3.374 3.374 0 0014 18.469V19a2 2 0 11-4 0v-.531c0-.895-.356-1.754-.988-2.386l-.548-.547z" />
          </svg>
          AI Agents ({agents.length})
        </h3>
        <div className="flex gap-2">
          <button
            onClick={() => {
              const name = prompt('Agent name:') || 'New Agent';
              const type = confirm('Coding agent?') ? 'CODING' : 'RESEARCH';
              api.createAgent(name, type, 1, 1000).then(fetchAgents);
            }}
            className="px-3 py-1 text-xs bg-purple-600 hover:bg-purple-700 rounded text-white disabled:opacity-50"
            disabled={actionLoading !== null}
          >
            + Create Agent
          </button>
        </div>
      </div>

      {agents.length === 0 ? (
        <div className="flex-1 flex items-center justify-center">
          <div className="text-center">
            <svg className="w-12 h-12 text-gray-600 mx-auto mb-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9.663 17h4.673M12 3v1m6.364 1.636l-.707.707M21 12h-1M4 12H3m3.343-5.657l-.707-.707m2.828 9.9a5 5 0 117.072 0l-.548.547A3.374 3.374 0 0014 18.469V19a2 2 0 11-4 0v-.531c0-.895-.356-1.754-.988-2.386l-.548-.547z" />
            </svg>
            <p className="text-gray-500">No AI agents running</p>
            <p className="text-xs text-gray-600 mt-1">Click "Create Agent" to add one</p>
          </div>
        </div>
      ) : (
        <div className="flex-1 overflow-y-auto space-y-3">
          {agents.map((agent) => {
            const details = agentDetails.find(d => d.pid === agent.pid);
            return (
              <div
                key={agent.pid}
                className="bg-gray-700 rounded-lg border border-gray-600 p-4 transition-all"
              >
                <div className="flex items-start justify-between mb-3">
                  <div className="flex items-center gap-3">
                    <div className="w-10 h-10 bg-purple-600 rounded-full flex items-center justify-center">
                      <svg className="w-6 h-6 text-white" fill="currentColor" viewBox="0 0 24 24">
                        <path d="M12 4.5C7 4.5 2.73 7.61 1 12c1.73 4.39 6 7.5 11 7.5 5 0 9.27-3.11 11-7.5C23 7.61 18.27 4.5 12 4.5zm0 1.5c1.38 0 2.5.74 3 1.85 1.23 3.18 3.71 5.5 7 5.5s5.77-2.32 7-5.5c-.5-1.11-1.62-1.85-3-1.85zM12 6.5c-.83 0-1.5.67-1.5 1.5S11.17 9.5 12 9.5s1.5-.67 1.5-1.5S12.83 6.5 12 6.5zm3 9.5c0-.83.67-1.5 1.5-1.5s1.5.67 1.5 1.5S17.83 17.5 17 17.5H7c-.83 0-1.5-.67-1.5-1.5S6.17 14.5 7 14.5zm0-9.5C9.33 6.5 10 5.83 10 5.83s-.67-1.5-1.5-1.5S7 5.83 7 5.83S7.67 6.5 7 6.5z"/>
                      </svg>
                    </div>
                    <div>
                      <div className="font-medium text-white flex items-center gap-2">
                        {agent.name}
                        {details?.agentState && (
                          <span className={`px-2 py-0.5 text-xs rounded ${getAgentStateColor(details.agentState)}`}>
                            {details.agentState}
                          </span>
                        )}
                      </div>
                      <div className="text-xs text-gray-400">PID: {agent.pid} • {agent.agentId}</div>
                    </div>
                  </div>
                  <div className="flex items-center gap-2">
                    <span className={`px-2 py-0.5 text-xs rounded ${getStateColor(agent.state)}`}>
                      {agent.state}
                    </span>
                    <button
                      onClick={() => handleViewCheckpoints(agent.agentId)}
                      className="px-2 py-1 text-xs bg-purple-600 hover:bg-purple-700 rounded text-white"
                    >
                      {showCheckpoints === agent.agentId ? 'Hide' : 'View'} Checkpoints
                    </button>
                  </div>
                </div>

                <div className="grid grid-cols-2 gap-4 mb-3">
                  <div>
                    <div className="text-xs text-gray-400">Task</div>
                    <div className="text-sm text-white truncate">{agent.task || 'N/A'}</div>
                  </div>
                  <div>
                    <div className="text-xs text-gray-400">Priority</div>
                    <div className="text-sm text-white">{agent.priority}</div>
                  </div>
                </div>

{details && (
                  <div className="grid grid-cols-3 gap-4 mb-3">
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">Tokens Used</div>
                      <div className="text-sm font-medium text-white">{details.tokenUsage.used}</div>
                    </div>
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">Budget</div>
                      <div className="text-sm font-medium text-white">{details.tokenUsage.budget}</div>
                    </div>
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">Usage %</div>
                      <div className="text-sm font-medium text-white">
                        {getTokenPercentage(details.tokenUsage)}%
                      </div>
                    </div>
                  </div>
                )}

                <div className="mb-3">
                  <div className="text-xs text-gray-400 mb-1">Permissions</div>
                  <div className="flex flex-wrap gap-1">
                    {Object.entries(agent.permissions || {}).map(([key, value]) => (
                      <span
                        key={key}
                        className={`px-2 py-0.5 text-xs rounded ${value ? 'bg-green-100 text-green-800' : 'bg-red-100 text-red-800'}`}
                      >
                        {key}: {value ? '✓' : '✗'}
                      </span>
                    ))}
                  </div>
                </div>

                {details && (
                  <div className="grid grid-cols-4 gap-3 mb-3 text-center">
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">CPU Cycles</div>
                      <div className="text-sm font-medium text-white">{details.cpuTime}</div>
                    </div>
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">Memory (pages)</div>
                      <div className="text-sm font-medium text-white">{details.memoryUsage}</div>
                    </div>
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">IPC Sent</div>
                      <div className="text-sm font-medium text-white">{details.ipcSent}</div>
                    </div>
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">IPC Recv</div>
                      <div className="text-sm font-medium text-white">{details.ipcReceived}</div>
                    </div>
                  </div>
                )}

                {details && (
                  <div className="grid grid-cols-3 gap-3 mb-3 text-center">
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">Checkpoints</div>
                      <div className="text-sm font-medium text-white">{details.checkpointCount}</div>
                    </div>
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">Failures</div>
                      <div className="text-sm font-medium text-white">{details.failureCount}</div>
                    </div>
                    <div className="bg-gray-800 rounded p-2">
                      <div className="text-xs text-gray-400">Recoveries</div>
                      <div className="text-sm font-medium text-white">{details.recoveryCount}</div>
                    </div>
                  </div>
                )}

                <div className="flex flex-wrap gap-2 mb-3">
                  <button
                    onClick={() => handleCreateCheckpoint(agent.agentId)}
                    disabled={actionLoading === agent.agentId || agent.state === 'FAILED' || agent.state === 'TERMINATED'}
                    className="flex-1 px-2 py-1 text-xs bg-green-600 hover:bg-green-700 disabled:opacity-50 rounded text-white"
                  >
                    {actionLoading === agent.agentId ? '⏳' : '💾'} Checkpoint
                  </button>
                  <button
                    onClick={() => handleFailAgent(agent.agentId)}
                    disabled={actionLoading === agent.agentId || agent.state === 'FAILED' || agent.state === 'TERMINATED'}
                    className="flex-1 px-2 py-1 text-xs bg-red-600 hover:bg-red-700 disabled:opacity-50 rounded text-white"
                  >
                    {actionLoading === agent.agentId ? '⏳' : '💥'} Fail
                  </button>
                  <button
                    onClick={() => handleRecoverAgent(agent.agentId, details?.lastCheckpointId)}
                    disabled={actionLoading === agent.agentId || agent.state !== 'FAILED' || !details?.lastCheckpointId}
                    className="flex-1 px-2 py-1 text-xs bg-blue-600 hover:bg-blue-700 disabled:opacity-50 rounded text-white"
                  >
                    {actionLoading === agent.agentId ? '⏳' : '🔄'} Recover
                  </button>
                </div>

                <div className="flex flex-wrap gap-1">
                  {Object.entries(agent.permissions || {}).map(([key, value]) => (
                    <button
                      key={key}
                      onClick={() => handleRequestTool(agent.agentId, key)}
                      disabled={actionLoading === `${agent.agentId}-${key}`}
                      className={`px-2 py-0.5 text-xs rounded ${value ? 'bg-green-100 text-green-800' : 'bg-red-100 text-red-800'} hover:opacity-80 disabled:opacity-50`}
                    >
                      {key} {value ? '✓' : '✗'}
                    </button>
                  ))}
                </div>

                {showCheckpoints === agent.agentId && agentCheckpoints.length > 0 && (
                  <div className="mt-3 pt-3 border-t border-gray-600">
                    <div className="text-xs font-medium text-gray-400 mb-2">Checkpoints ({agentCheckpoints.length})</div>
                    <div className="space-y-2 max-h-40 overflow-y-auto">
                      {agentCheckpoints.map((cp) => (
                        <div
                          key={cp.checkpointId}
                          className="bg-gray-800 rounded p-2 text-xs"
                        >
                          <div className="flex justify-between">
                            <span className="text-purple-400">{cp.checkpointId}</span>
                            <span className="text-gray-400">Cycle {cp.createdCycle}</span>
                          </div>
                          <div className="text-gray-300">{cp.progress}</div>
                          <div className="flex justify-between mt-1">
                            <button
                              onClick={() => handleRecoverAgent(agent.agentId, cp.checkpointId)}
                              disabled={actionLoading === agent.agentId || agent.state !== 'FAILED'}
                              className="text-xs text-blue-400 hover:underline"
                            >
                              Recover from this
                            </button>
                            <span className="text-gray-500">PC: {cp.cpuContext.pc}</span>
                          </div>
                        </div>
                      ))}
                    </div>
                  </div>
                )}
              </div>
            );
          })}
        </div>
      )}
    </div>
  );
}

export default AIAgentPanel;