import { useEffect, useState } from 'react';
import { api } from '../../services/api';
import type { Agent } from '../../types/os';

interface AIAgentPanelProps {
  autoRefresh?: boolean;
  refreshInterval?: number;
}

export function AIAgentPanel({ autoRefresh = true, refreshInterval = 3000 }: AIAgentPanelProps) {
  const [agents, setAgents] = useState<Agent[]>([]);

  const fetchData = async () => {
    try {
      const agentsRes = await api.getAgents();
      if (agentsRes.success && agentsRes.data) setAgents(agentsRes.data);
    } catch (error) {
      console.error('Failed to fetch agents:', error);
    }
  };

  useEffect(() => {
    fetchData();
    if (autoRefresh) {
      const interval = setInterval(fetchData, refreshInterval);
      return () => clearInterval(interval);
    }
}, [autoRefresh, refreshInterval]);

  return (
    <div className="bg-gray-800 rounded-lg p-4 h-full flex flex-col">
      <h3 className="text-lg font-semibold text-white mb-4 flex items-center gap-2">
        <svg className="w-5 h-5 text-purple-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9.663 17h4.673M12 3v1m6.364 1.636l-.707.707M21 12h-1M4 12H3m3.343-5.657l-.707-.707m2.828 9.9a5 5 0 117.072 0l-.548.547A3.374 3.374 0 0014 18.469V19a2 2 0 11-4 0v-.531c0-.895-.356-1.754-.988-2.386l-.548-.547z" />
        </svg>
        AI Agents ({agents.length})
      </h3>

      {agents.length === 0 ? (
        <div className="flex-1 flex items-center justify-center">
          <p className="text-gray-500 text-center">No AI agents running</p>
        </div>
      ) : (
        <div className="flex-1 overflow-y-auto space-y-3">
          {agents.map((agent) => (
            <div
              key={agent.pid}
              className="bg-gray-700 rounded-lg border border-gray-600 p-4"
            >
              <div className="flex items-start justify-between mb-3">
                <div className="flex items-center gap-3">
                  <div className="w-10 h-10 bg-purple-600 rounded-full flex items-center justify-center">
                    <svg className="w-6 h-6 text-white" fill="currentColor" viewBox="0 0 24 24">
                      <path d="M12 4.5C7 4.5 2.73 7.61 1 12c1.73 4.39 6 7.5 11 7.5 5 0 9.27-3.11 11-7.5C23 7.61 18.27 4.5 12 4.5zm0 1.5c1.38 0 2.5.74 3 1.85 1.23 3.18 3.71 5.5 7 5.5s5.77-2.32 7-5.5c-.5-1.11-1.62-1.85-3-1.85zM12 6.5c-.83 0-1.5.67-1.5 1.5S11.17 9.5 12 9.5s1.5-.67 1.5-1.5S12.83 6.5 12 6.5zm3 9.5c0-.83.67-1.5 1.5-1.5s1.5.67 1.5 1.5S17.83 17.5 17 17.5H7c-.83 0-1.5-.67-1.5-1.5S6.17 14.5 7 14.5zm0-9.5C9.33 6.5 10 5.83 10 5.83s-.67-1.5-1.5-1.5S7 5.83 7 5.83S7.67 6.5 7 6.5z"/>
                    </svg>
                  </div>
                  <div>
                    <div className="font-medium text-white">{agent.name}</div>
                    <div className="text-xs text-gray-400">PID: {agent.pid} • {agent.agentId}</div>
                  </div>
                </div>
                <span className={`px-2 py-0.5 text-xs rounded ${(agent.state === 'RUNNING' ? 'bg-green-100 text-green-800' : agent.state === 'READY' ? 'bg-blue-100 text-blue-800' : agent.state === 'WAITING' ? 'bg-yellow-100 text-yellow-800' : agent.state === 'FAILED' ? 'bg-red-100 text-red-800' : 'bg-gray-100 text-gray-800')}`}>
                  {agent.state}
                </span>
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

              <div className="grid grid-cols-3 gap-4 mb-3">
                <div className="bg-gray-800 rounded p-2">
                  <div className="text-xs text-gray-400">Tokens Used</div>
                  <div className="text-sm font-medium text-white">{agent.tokenUsage?.used || 0}</div>
                </div>
                <div className="bg-gray-800 rounded p-2">
                  <div className="text-xs text-gray-400">Budget</div>
                  <div className="text-sm font-medium text-white">{agent.tokenUsage?.budget || 1000}</div>
                </div>
                <div className="bg-gray-800 rounded p-2">
                  <div className="text-xs text-gray-400">Usage %</div>
                  <div className="text-sm font-medium text-white">
                    {agent.tokenUsage?.budget ? Math.round((agent.tokenUsage.used / agent.tokenUsage.budget) * 100) : 0}%
                  </div>
                </div>
              </div>

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
          ))}
        </div>
      )}
    </div>
  );
}

export default AIAgentPanel;