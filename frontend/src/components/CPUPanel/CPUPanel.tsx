// CPUPanel Component - CPU State per docs/13_GUI_Architecture.md section 20
import type { CPUState } from '../../types/os';

interface CPUPanelProps {
  cpu: CPUState | null;
}

const REGISTERS: { key: keyof Pick<CPUState, 'pc' | 'ir' | 'acc' | 'mar' | 'mbr'>; label: string }[] = [
  { key: 'pc', label: 'PC' },
  { key: 'ir', label: 'IR' },
  { key: 'acc', label: 'ACC' },
  { key: 'mar', label: 'MAR' },
  { key: 'mbr', label: 'MBR' },
];

const FLAGS: { key: keyof CPUState['flags']; label: string; on: string }[] = [
  { key: 'zero', label: 'Z', on: 'text-blue-300 border-blue-500/60 bg-blue-900/30' },
  { key: 'negative', label: 'N', on: 'text-red-300 border-red-500/60 bg-red-900/30' },
  { key: 'carry', label: 'C', on: 'text-yellow-300 border-yellow-500/60 bg-yellow-900/30' },
  { key: 'overflow', label: 'V', on: 'text-purple-300 border-purple-500/60 bg-purple-900/30' },
  { key: 'error', label: 'E', on: 'text-red-200 border-red-400/70 bg-red-800/40 animate-pulse' },
];

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
      <div className="px-4 py-2.5 border-b border-gray-700 bg-gray-900 flex items-center justify-between">
        <h3 className="font-medium text-gray-100 text-sm">CPU State</h3>
        {/* Flags inline in the header to keep the panel compact */}
        <div className="flex items-center gap-1">
          {FLAGS.map(f => (
            <span
              key={f.key}
              title={f.key}
              className={`w-5 h-5 flex items-center justify-center rounded border text-[10px] font-mono font-bold ${
                cpu.flags[f.key] ? f.on : 'text-gray-600 border-gray-700'
              }`}
            >
              {f.label}
            </span>
          ))}
        </div>
      </div>

      {/* Registers as a single tight row */}
      <div className="grid grid-cols-5 divide-x divide-gray-700 flex-1">
        {REGISTERS.map(reg => (
          <div key={reg.key} className="px-2 py-3 text-center">
            <div className="text-[10px] uppercase tracking-wider text-gray-500">{reg.label}</div>
            <div className="font-mono text-sm text-gray-100 mt-0.5 truncate" title={String(cpu[reg.key])}>
              {cpu[reg.key]}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}
