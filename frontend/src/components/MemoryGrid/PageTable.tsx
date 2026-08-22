// PageTable Component - Virtual-to-Physical mappings per docs/13_GUI_Architecture.md section 24
// One process at a time with prev/next toggles to keep the panel compact.
import { useEffect, useState } from 'react';
import type { PageTable } from '../../types/os';

interface PageTableProps {
  pageTables: PageTable[];
}

export function PageTableComponent({ pageTables }: PageTableProps) {
  const [index, setIndex] = useState(0);

  // Clamp when the workload changes (reset / new pids).
  useEffect(() => {
    if (index >= pageTables.length) setIndex(0);
  }, [pageTables.length, index]);

  if (pageTables.length === 0) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg flex flex-col h-full overflow-hidden">
        <div className="px-4 py-2.5 border-b border-gray-700 bg-gray-900">
          <h3 className="font-medium text-gray-100 text-sm">Page Tables</h3>
        </div>
        <p className="text-gray-500 text-center py-8 text-sm px-4">
          No page tables yet — pages appear after processes fault in
        </p>
      </div>
    );
  }

  const clamped = Math.min(index, pageTables.length - 1);
  const table = pageTables[clamped];
  const resident = table.entries.filter(e => e.valid && e.frame !== null && e.frame >= 0).length;

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg flex flex-col h-full overflow-hidden">
      <div className="px-2 py-2 border-b border-gray-700 bg-gray-900 flex items-center justify-between gap-1">
        <button
          onClick={() => setIndex((clamped - 1 + pageTables.length) % pageTables.length)}
          disabled={pageTables.length <= 1}
          className="w-6 h-6 rounded bg-gray-800 border border-gray-600 text-gray-300 hover:bg-gray-700 disabled:opacity-40 disabled:cursor-not-allowed text-sm leading-none"
          title="Previous process"
        >
          ‹
        </button>
        <div className="text-center leading-tight">
          <h3 className="font-medium text-gray-100 text-sm">
            Page Table <span className="font-mono">P{table.processId}</span>
          </h3>
          <p className="text-[10px] text-gray-400 font-mono">
            {clamped + 1} / {pageTables.length} · {resident}/{table.entries.length} resident
          </p>
        </div>
        <button
          onClick={() => setIndex((clamped + 1) % pageTables.length)}
          disabled={pageTables.length <= 1}
          className="w-6 h-6 rounded bg-gray-800 border border-gray-600 text-gray-300 hover:bg-gray-700 disabled:opacity-40 disabled:cursor-not-allowed text-sm leading-none"
          title="Next process"
        >
          ›
        </button>
      </div>

      <div className="overflow-y-auto flex-1 min-h-0">
        <table className="w-full text-xs">
          <thead className="bg-gray-900/70 sticky top-0">
            <tr>
              <th className="px-2 py-1.5 text-left font-medium text-gray-400 w-12">Page</th>
              <th className="px-2 py-1.5 text-left font-medium text-gray-400 w-12">Frame</th>
              <th className="px-2 py-1.5 text-center font-medium text-gray-400 w-10">V</th>
              <th className="px-2 py-1.5 text-center font-medium text-gray-400 w-10">D</th>
              <th className="px-2 py-1.5 text-center font-medium text-gray-400 w-10">R</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-gray-700/60">
            {table.entries.map(entry => (
              <tr key={entry.page} className={entry.valid ? '' : 'opacity-50'}>
                <td className="px-2 py-1 font-mono text-gray-300">{entry.page}</td>
                <td className="px-2 py-1 font-mono text-gray-300">
                  {entry.frame !== null && entry.frame >= 0 ? entry.frame : '—'}
                </td>
                <td className={`px-2 py-1 text-center font-mono ${entry.valid ? 'text-green-400' : 'text-red-400'}`}>
                  {entry.valid ? 1 : 0}
                </td>
                <td className={`px-2 py-1 text-center font-mono ${entry.modified ? 'text-yellow-400' : 'text-gray-600'}`}>
                  {entry.modified ? 1 : 0}
                </td>
                <td className={`px-2 py-1 text-center font-mono ${entry.referenced ? 'text-blue-400' : 'text-gray-600'}`}>
                  {entry.referenced ? 1 : 0}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}
