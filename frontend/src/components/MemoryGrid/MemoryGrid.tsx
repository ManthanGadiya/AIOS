// MemoryGrid Component - Physical Memory Visualization per docs/13_GUI_Architecture.md section 23
import type { MemoryFrame } from '../../types/os';

interface MemoryGridProps {
  frames: MemoryFrame[];
}

const CELL_COLORS = [
  'bg-blue-500', 'bg-green-500', 'bg-purple-500', 'bg-orange-500',
  'bg-pink-500', 'bg-cyan-500', 'bg-amber-500', 'bg-rose-500',
];

function cellColor(owner: string): string {
  if (owner === 'Free') return '';
  let hash = 0;
  for (let i = 0; i < owner.length; i++) {
    hash = owner.charCodeAt(i) + ((hash << 5) - hash);
  }
  return CELL_COLORS[Math.abs(hash) % CELL_COLORS.length];
}

export function MemoryGrid({ frames }: MemoryGridProps) {
  const used = frames.filter(f => f.owner !== 'Free').length;

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg flex flex-col h-full overflow-hidden">
      <div className="px-4 py-2.5 border-b border-gray-700 bg-gray-900 flex items-center justify-between">
        <h3 className="font-medium text-gray-100 text-sm">Physical Memory</h3>
        {/* Inline legend + usage keeps the panel compact */}
        <span className="text-[10px] text-gray-400 font-mono">
          {used}/{frames.length} frames
        </span>
      </div>

      <div className="p-3 overflow-y-auto flex-1 min-h-0">
        <div className="grid grid-cols-8 xl:grid-cols-16 gap-1">
          {frames.map(frame => (
            <div
              key={frame.frameId}
              title={`Frame ${frame.frameId}: ${frame.owner === 'Free' ? 'free' : `${frame.owner} page ${frame.pageId}`}`}
              className={`h-7 rounded-sm border text-[9px] font-mono flex items-center justify-center ${
                frame.owner === 'Free'
                  ? 'bg-gray-900/70 border-gray-700 text-gray-600'
                  : `${cellColor(frame.owner)} border-gray-600 text-white`
              }`}
            >
              {frame.owner === 'Free' ? frame.frameId : frame.owner.replace('P', '')}
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
