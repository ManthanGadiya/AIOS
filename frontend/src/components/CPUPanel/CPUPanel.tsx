// CPUPanel Component - CPU State per docs/13_GUI_Architecture.md section 20
import type { CPUState } from '../../types/os';

interface CPUPanelProps {
  cpu: CPUState | null;
}

const FLAG_LABELS = [
  { key: 'zero', label: 'Z', color: 'text-blue-400' },
  { key: 'negative', label: 'N', color: 'text-red-400' },
  { key: 'carry', label: 'C', color: 'text-yellow-400' },
  { key: 'overflow', label: 'V', color: 'text-purple-400' },
  { key: 'error', label: 'E', color: 'text-red-500' },
];

const REGISTERS = [
  { key: 'pc', label: 'PC', format: (v: number) => `0x${v.toString(16).toUpperCase().padStart(4, '0')}` },
  { key: 'ir', label: 'IR', format: (v: number) => `0x${v.toString(16).toUpperCase().padStart(4, '0')}` },
  { key: 'acc', label: 'ACC', format: (v: number) => v.toString() },
  { key: 'mar', label: 'MAR', format: (v: number) => `0x${v.toString(16).toUpperCase().padStart(4, '0')}` },
  { key: 'mbr', label: 'MBR', format: (v: number) => `0x${v.toString(16).toUpperCase().padStart(4, '0')}` },
] as const;

export function CPUPanel({ cpu }: CPUPanelProps) {
  if (!cpu) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center text-gray-500">
        CPU state unavailable
      </div>
    );
  }

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg">
      <div className="px-4 py-3 border-b border-gray-700 bg-gray-900 rounded-t-lg">
        <h3 className="font-medium text-gray-100 flex items-center gap-2">
          <svg className="w-5 h-5 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z" />
          </svg>
          CPU State
        </h3>
      </div>
      
      <div className="p-4 space-y-3">
        <div className="grid grid-cols-2 gap-3">
          {REGISTERS.map(reg => (
            <div key={reg.key} className="bg-gray-900 rounded p-3 border border-gray-700">
              <div className="text-xs text-gray-500 uppercase tracking-wide mb-1">{reg.label}</div>
              <div className="font-mono text-lg text-gray-100">{reg.format(cpu[reg.key as keyof CPUState] as number)}</div>
            </div>
          ))}
        </div>

        <div className="border-t border-gray-700 pt-3">
          <div className="text-xs text-gray-500 uppercase tracking-wide mb-2">FLAGS</div>
          <div className="flex flex-wrap gap-2">
            {FLAG_LABELS.map(flag => (
              <span
                key={flag.key}
                className={`px-2 py-1 rounded text-xs font-mono font-medium border ${
                  cpu.flags[flag.key as keyof CPUState['flags']]
                    ? `${flag.color} bg-${flag.color.replace('text-', '').replace('-400', '-900/30')} border-current`
                    : 'text-gray-500 border-gray-600'
                }`}
              >
                {flag.label}: {cpu.flags[flag.key as keyof CPUState['flags']] ? '1' : '0'}
              </span>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}