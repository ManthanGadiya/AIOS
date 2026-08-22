// SchedulerPanel Component - Scheduler state + Gantt timeline
// per docs/13_GUI_Architecture.md sections 27 and 29: current algorithm,
// running process, quantum, context switches, and a timeline rendered from
// the actual scheduling history so non-preemptive policies (FCFS / Priority)
// show visible progress instead of appearing frozen.
import type { SchedulerState } from '../../types/os';

interface SchedulerPanelProps {
  scheduler: SchedulerState | null;
}

const SEGMENT_COLORS = [
  'bg-blue-500', 'bg-green-500', 'bg-purple-500', 'bg-orange-500',
  'bg-pink-500', 'bg-cyan-500', 'bg-amber-500', 'bg-rose-500',
];

function colorForPid(pid: number): string {
  let hash = pid * 2654435761;
  hash = (hash ^ (hash >>> 16)) >>> 0;
  return SEGMENT_COLORS[hash % SEGMENT_COLORS.length];
}

export function SchedulerPanel({ scheduler }: SchedulerPanelProps) {
  if (!scheduler) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-6 text-center text-gray-500 text-sm h-full">
        Scheduler unavailable
      </div>
    );
  }

  const history = scheduler.schedulingHistory ?? [];
  const now = scheduler.currentTimeCycle ?? 0;

  // Gantt segments (docs/13 section 29): decision i owns the CPU from its own
  // cycle until the next decision (or the current cycle). Rendered oldest on
  // the left; short quanta get a minimum width so RR stays readable.
  const recent = history.slice(-14);
  const segments = recent.map((d, i) => {
    const nextCycle = i + 1 < recent.length ? recent[i + 1].cycle : Math.max(now, d.cycle);
    const duration = Math.max(1, Number(nextCycle) - Number(d.cycle));
    return { pid: d.selectedPid, start: d.cycle, duration };
  });

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg flex flex-col h-full overflow-hidden">
      <div className="px-4 py-2.5 border-b border-gray-700 bg-gray-900">
        <h3 className="font-medium text-gray-100 text-sm">Scheduler</h3>
      </div>

      <div className="p-3 space-y-3 flex-1 min-h-0 overflow-y-auto">
        {/* Current state line */}
        <div className="grid grid-cols-3 gap-2 text-center">
          <div className="bg-gray-900 rounded p-1.5 border border-gray-700">
            <div className="text-[9px] uppercase tracking-wide text-gray-500">Policy</div>
            <div className="text-xs font-mono text-gray-200 truncate" title={scheduler.currentAlgorithm}>
              {scheduler.currentAlgorithm.replace('_', ' ')}
            </div>
          </div>
          <div className="bg-gray-900 rounded p-1.5 border border-gray-700">
            <div className="text-[9px] uppercase tracking-wide text-gray-500">On CPU</div>
            <div className="text-xs font-mono text-green-400">
              {scheduler.currentProcess !== null && scheduler.currentProcess >= 0 ? `P${scheduler.currentProcess}` : 'idle'}
            </div>
          </div>
          <div className="bg-gray-900 rounded p-1.5 border border-gray-700">
            <div className="text-[9px] uppercase tracking-wide text-gray-500">Switches</div>
            <div className="text-xs font-mono text-gray-200">{scheduler.metrics?.contextSwitchCount ?? 0}</div>
          </div>
        </div>

        {/* Gantt timeline (docs/13 section 29) */}
        <div>
          <div className="flex items-center justify-between mb-1">
            <span className="text-[10px] uppercase tracking-wide text-gray-500">CPU Timeline</span>
            <span className="text-[10px] text-gray-500 font-mono">{history.length} decisions</span>
          </div>
          <div className="flex items-stretch gap-px h-7 bg-gray-900 rounded border border-gray-700 overflow-hidden">
            {segments.length === 0 && (
              <div className="flex-1 flex items-center justify-center text-[10px] text-gray-600">
                no scheduling decisions yet — press Start
              </div>
            )}
            {segments.map((seg, i) => (
              <div
                key={i}
                className={`${colorForPid(seg.pid)} flex items-center justify-center text-white text-[10px] font-mono overflow-hidden`}
                style={{ flexGrow: seg.duration, flexBasis: 0, minWidth: seg.duration > 8 ? '2rem' : '0.75rem' }}
                title={`P${seg.pid}: cycles ${seg.start}–${seg.start + seg.duration}`}
              >
                P{seg.pid}
              </div>
            ))}
          </div>
        </div>

        {/* Latest decisions with reasons */}
        <div>
          <span className="text-[10px] uppercase tracking-wide text-gray-500">Recent decisions</span>
          <div className="mt-1 space-y-0.5">
            {[...recent].reverse().slice(0, 4).map((d, i) => (
              <div key={i} className="text-[10px] font-mono text-gray-400 truncate" title={d.reason}>
                <span className="text-gray-600">@{d.cycle}</span>{' '}
                <span className={colorForPid(d.selectedPid).replace('bg-', 'text-')}>P{d.selectedPid}</span>
                {' ← '}
                {d.reason}
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}
