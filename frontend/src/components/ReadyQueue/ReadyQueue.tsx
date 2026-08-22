// ReadyQueue Component - Scheduler Ready Queue visualization per docs/13_GUI_Architecture.md section 26
import type { Process } from '../../types/os';

interface ReadyQueueProps {
  readyQueue: number[]; // PIDs in order
  processes: Process[]; // All processes for lookup
  runningPid: number | null;
}

export function ReadyQueue({ readyQueue, processes, runningPid }: ReadyQueueProps) {
  const processMap = new Map(processes.map(p => [p.pid, p]));
  
  const queueItems = [
    ...(runningPid !== null ? [{ pid: runningPid, isRunning: true }] : []),
    ...readyQueue.map(pid => ({ pid, isRunning: false })),
  ];

  if (queueItems.length === 0) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center text-gray-500">
        Ready queue empty
      </div>
    );
  }

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg">
      <div className="px-4 py-3 border-b border-gray-700 bg-gray-900 rounded-t-lg">
        <h3 className="font-medium text-gray-100 flex items-center gap-2">
          <svg className="w-5 h-5 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 6h16M4 12h16M4 18h16" />
          </svg>
          Ready Queue
        </h3>
      </div>
      
      <div className="p-4">
        <div className="flex flex-wrap items-center gap-2">
          {queueItems.map((item, index) => {
            const proc = processMap.get(item.pid);
            return (
              <div
                key={`${item.pid}-${index}`}
                className={`flex items-center gap-1 px-3 py-2 rounded border transition-all ${
                  item.isRunning
                    ? 'bg-green-900/30 border-green-500 ring-1 ring-green-500/30'
                    : 'bg-gray-800 border-gray-600 hover:border-gray-500'
                }`}
              >
                {index > 0 && !item.isRunning && (
                  <svg className="w-4 h-4 text-gray-500 flex-shrink-0" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M13 7l5 5m0 0l-5 5m5-5H6" />
                  </svg>
                )}
                <div className={`px-2 py-1 rounded text-xs font-medium font-mono ${
                  item.isRunning ? 'bg-green-500 text-white' : 'bg-gray-700 text-gray-300'
                }`}>
                  {proc ? proc.name : `P${item.pid}`}
                </div>
                <div className="text-xs text-gray-500 hidden sm:block">PID {item.pid}</div>
              </div>
            );
          })}
        </div>
        
        <div className="mt-3 pt-3 border-t border-gray-700">
          <div className="flex items-center gap-4 text-xs text-gray-400">
            <span>Total in queue: <span className="text-gray-100 font-mono">{readyQueue.length}</span></span>
            <span>Running: <span className="text-green-400 font-mono">{runningPid !== null ? `P${runningPid}` : 'None'}</span></span>
          </div>
        </div>
      </div>
    </div>
  );
}