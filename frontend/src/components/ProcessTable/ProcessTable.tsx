// ProcessTable Component - Process List per docs/13_GUI_Architecture.md section 17
import type { Process, ProcessState, ProcessType } from '../../types/os';

const STATE_COLORS: Record<ProcessState, string> = {
  NEW: 'bg-gray-500',
  READY: 'bg-blue-500',
  RUNNING: 'bg-green-500',
  WAITING: 'bg-yellow-500',
  TERMINATED: 'bg-gray-400',
  FAILED: 'bg-red-500',
};

const STATE_LABELS: Record<ProcessState, string> = {
  NEW: 'NEW',
  READY: 'READY',
  RUNNING: 'RUNNING',
  WAITING: 'WAITING',
  TERMINATED: 'TERM',
  FAILED: 'FAILED',
};

const TYPE_LABELS: Record<ProcessType, string> = {
  NORMAL: 'Process',
  AI_AGENT: 'Agent',
};

interface ProcessTableProps {
  processes: Process[];
  runningPid: number | null;
}

export function ProcessTable({ processes, runningPid }: ProcessTableProps) {
  if (processes.length === 0) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center text-gray-500">
        No processes running
      </div>
    );
  }

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg overflow-hidden">
      <div className="overflow-x-auto">
        <table className="w-full text-sm">
          <thead className="bg-gray-900 border-b border-gray-700">
            <tr>
              <th className="px-3 py-2 text-left font-medium text-gray-400">PID</th>
              <th className="px-3 py-2 text-left font-medium text-gray-400">Type</th>
              <th className="px-3 py-2 text-left font-medium text-gray-400">Name</th>
              <th className="px-3 py-2 text-left font-medium text-gray-400">State</th>
              <th className="px-3 py-2 text-left font-medium text-gray-400">Priority</th>
              <th className="px-3 py-2 text-left font-medium text-gray-400">Memory</th>
              <th className="px-3 py-2 text-left font-medium text-gray-400">CPU Time</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-gray-700">
            {processes.map(proc => (
              <tr 
                key={proc.pid} 
                className={`hover:bg-gray-700/50 transition-colors ${proc.pid === runningPid ? 'bg-green-900/20' : ''}`}
              >
                <td className="px-3 py-2 font-mono text-gray-300">{proc.pid}</td>
                <td className="px-3 py-2">
                  <span className={`px-2 py-0.5 rounded text-xs font-medium ${
                    proc.type === 'AI_AGENT' ? 'bg-purple-900/30 text-purple-300' : 'bg-blue-900/30 text-blue-300'
                  }`}>
                    {TYPE_LABELS[proc.type]}
                  </span>
                </td>
                <td className="px-3 py-2 text-gray-100 font-mono">{proc.name}</td>
                <td className="px-3 py-2">
                  <span className={`inline-flex items-center gap-1.5 px-2 py-0.5 rounded text-xs font-medium ${STATE_COLORS[proc.state]} text-white`}>
                    <span className="w-1.5 h-1.5 rounded-full bg-current" />
                    {STATE_LABELS[proc.state]}
                  </span>
                </td>
                <td className="px-3 py-2 text-gray-300 font-mono">{proc.priority}</td>
                <td className="px-3 py-2 text-gray-300 font-mono">{proc.memory} MB</td>
                <td className="px-3 py-2 text-gray-400 font-mono">{proc.cpuTime}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}