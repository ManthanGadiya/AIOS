// MemoryGrid Component - Physical Memory Visualization per docs/13_GUI_Architecture.md section 23
import type { MemoryFrame } from '../../types/os';

interface MemoryGridProps {
  frames: MemoryFrame[];
}

const PROCESS_COLORS = [
  'bg-blue-500', 'bg-green-500', 'bg-purple-500', 'bg-orange-500',
  'bg-pink-500', 'bg-cyan-500', 'bg-amber-500', 'bg-rose-500',
];

function getColorForOwner(owner: string): string {
  if (owner === 'Free') return 'bg-gray-700';
  let hash = 0;
  for (let i = 0; i < owner.length; i++) {
    hash = owner.charCodeAt(i) + ((hash << 5) - hash);
  }
  return PROCESS_COLORS[Math.abs(hash) % PROCESS_COLORS.length];
}

export function MemoryGrid({ frames }: MemoryGridProps) {
  if (frames.length === 0) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center text-gray-500">
        Memory data unavailable
      </div>
    );
  }

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg flex flex-col h-full overflow-hidden">
      <div className="px-4 py-2.5 border-b border-gray-700 bg-gray-900">
        <h3 className="font-medium text-gray-100 text-sm">Physical Memory ({frames.length} frames)</h3>
      </div>
      
      <div className="p-4 overflow-y-auto flex-1 min-h-0">
        <div className="grid grid-cols-4 lg:grid-cols-8 gap-1">
          {frames.map((frame) => (
            <div
              key={frame.frameId}
              className={`aspect-square rounded border font-mono text-xs flex items-center justify-center transition-colors ${
                frame.owner === 'Free' 
                  ? 'bg-gray-700 border-gray-600 text-gray-400' 
                  : `${getColorForOwner(frame.owner)} border-gray-600 text-white`
              }`}
              title={`Frame ${frame.frameId}: ${frame.owner}${frame.pageId !== undefined ? ` (Page ${frame.pageId})` : ''}`}
            >
              <div className="text-center">
                <div className="font-medium">{frame.owner === 'Free' ? '—' : frame.owner}</div>
                <div className="opacity-70">F{frame.frameId}</div>
              </div>
            </div>
          ))}
        </div>
        
        <div className="mt-3 flex flex-wrap gap-2">
          <div className="flex items-center gap-1.5 text-xs text-gray-400">
            <div className="w-4 h-4 rounded bg-gray-700 border border-gray-600" />
            <span>Free</span>
          </div>
          {Array.from(new Set(frames.filter(f => f.owner !== 'Free').map(f => f.owner))).map(owner => (
            <div key={owner} className="flex items-center gap-1.5 text-xs text-gray-400">
              <div className="w-4 h-4 rounded" style={{ backgroundColor: getColorForOwner(owner).replace('bg-', '#') }} />
              <span>{owner}</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}