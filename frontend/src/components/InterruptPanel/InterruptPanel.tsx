// InterruptPanel Component - Interrupts per docs/13_GUI_Architecture.md section 30
import type { InterruptEvent, InterruptType, InterruptStatus } from '../../types/os';

const TYPE_COLORS: Record<InterruptType, string> = {
  TIMER: 'bg-blue-900/30 text-blue-400',
  PAGE_FAULT: 'bg-red-900/30 text-red-400',
  SYSTEM_CALL: 'bg-purple-900/30 text-purple-400',
  IO_COMPLETE: 'bg-green-900/30 text-green-400',
  IPC_EVENT: 'bg-pink-900/30 text-pink-400',
  ERROR: 'bg-red-900/30 text-red-400',
};

const STATUS_COLORS: Record<InterruptStatus, string> = {
  PENDING: 'bg-yellow-900/30 text-yellow-400',
  SERVICING: 'bg-blue-900/30 text-blue-400',
  COMPLETED: 'bg-green-900/30 text-green-400',
};

const PRIORITY_ORDER: InterruptType[] = ['ERROR', 'PAGE_FAULT', 'SYSTEM_CALL', 'IO_COMPLETE', 'TIMER', 'IPC_EVENT'];

interface InterruptPanelProps {
  interrupts: InterruptEvent[];
}

export function InterruptPanel({ interrupts }: InterruptPanelProps) {
  // Sort by priority then timestamp
  const sortedInterrupts = [...interrupts].sort((a, b) => {
    const priorityDiff = PRIORITY_ORDER.indexOf(a.type) - PRIORITY_ORDER.indexOf(b.type);
    if (priorityDiff !== 0) return priorityDiff;
    return new Date(b.timestamp).getTime() - new Date(a.timestamp).getTime();
  });

  if (interrupts.length === 0) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-6 text-center text-gray-500 text-sm h-full">
        No interrupts yet
      </div>
    );
  }

  const stats = {
    total: interrupts.length,
    handled: interrupts.filter(i => i.status === 'COMPLETED').length,
    pending: interrupts.filter(i => i.status === 'PENDING').length,
    pageFaults: interrupts.filter(i => i.type === 'PAGE_FAULT').length,
    timer: interrupts.filter(i => i.type === 'TIMER').length,
  };

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg flex flex-col h-full overflow-hidden">
      <div className="px-4 py-2.5 border-b border-gray-700 bg-gray-900">
        <h3 className="font-medium text-gray-100 text-sm">Interrupts</h3>
      </div>
      
      <div className="p-4 overflow-y-auto flex-1 min-h-0">
        {/* Stats Row */}
        <div className="grid grid-cols-5 gap-2 mb-3">
          <StatBox label="Total" value={stats.total} />
          <StatBox label="Handled" value={stats.handled} />
          <StatBox label="Pending" value={stats.pending} />
          <StatBox label="Faults" value={stats.pageFaults} />
          <StatBox label="Timer" value={stats.timer} />
        </div>

        {/* Interrupt Table */}
        <div className="overflow-x-auto">
          <table className="w-full text-sm">
            <thead className="bg-gray-900 border-b border-gray-700 sticky top-0">
              <tr>
                <th className="px-3 py-2 text-left font-medium text-gray-400">Time</th>
                <th className="px-3 py-2 text-left font-medium text-gray-400">Type</th>
                <th className="px-3 py-2 text-left font-medium text-gray-400">Description</th>
                <th className="px-3 py-2 text-left font-medium text-gray-400">Status</th>
                <th className="px-3 py-2 text-left font-medium text-gray-400">PID</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-gray-700">
              {sortedInterrupts.map(interrupt => (
                <tr key={interrupt.id} className="hover:bg-gray-700/50 transition-colors">
                  <td className="px-3 py-2 font-mono text-gray-400">{interrupt.timestamp}</td>
                  <td className="px-3 py-2">
                    <span className={`px-2 py-0.5 rounded text-xs font-medium ${TYPE_COLORS[interrupt.type] || 'bg-gray-700 text-gray-300'}`}>
                      {interrupt.type}
                    </span>
                  </td>
                  <td className="px-3 py-2 text-gray-200">{interrupt.description}</td>
                  <td className="px-3 py-2">
                    <span className={`px-2 py-0.5 rounded text-xs font-medium ${STATUS_COLORS[interrupt.status] || 'bg-gray-700 text-gray-300'}`}>
                      {interrupt.status}
                    </span>
                  </td>
                  <td className="px-3 py-2 font-mono text-gray-400">
                    {interrupt.pid !== undefined ? `P${interrupt.pid}` : '—'}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}

function StatBox({ label, value }: { label: string; value: number; color?: string }) {
  return (
    <div className="bg-gray-900 border border-gray-700 rounded p-3 text-center">
      <div className="text-2xl font-bold font-mono text-gray-100">{value}</div>
      <div className="text-xs text-gray-400 mt-1">{label}</div>
    </div>
  );
}