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
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-6 text-center text-gray-500 text-sm h-full">
        CPU state unavailable
      </div>
    );
  }

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg flex flex-col h-full overflow-hidden">
      <div className="px-4 py-2.5 border-b border-gray-700 bg-gray-900">
        <h3 className="font-medium text-gray-100 text-sm">CPU State</h3>
      </div>
      
      <div className="p-4 space-y-3 overflow-y-auto">
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