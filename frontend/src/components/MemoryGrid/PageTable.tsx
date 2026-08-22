// PageTable Component - Virtual-to-Physical mappings per docs/13_GUI_Architecture.md section 24
import type { PageTable } from '../../types/os';

interface PageTableProps {
  pageTables: PageTable[];
}

export function PageTableComponent({ pageTables }: PageTableProps) {
  if (pageTables.length === 0) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center text-gray-500">
        Page tables unavailable
      </div>
    );
  }

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg flex flex-col h-full overflow-hidden">
      <div className="px-4 py-2.5 border-b border-gray-700 bg-gray-900">
        <h3 className="font-medium text-gray-100 text-sm">Page Tables</h3>
      </div>
      
      <div className="p-4 space-y-4 overflow-y-auto flex-1 min-h-0">
        {pageTables.length === 0 && (
          <p className="text-gray-500 text-center py-6 text-sm">No page tables — pages appear after processes fault in</p>
        )}
        {pageTables.map(table => (
          <div key={table.processId} className="border border-gray-700 rounded overflow-hidden">
            <div className="bg-gray-900 px-3 py-2 border-b border-gray-700">
              <span className="text-sm font-medium text-gray-300">Process {table.processId} — Page Table</span>
            </div>
            <div className="overflow-x-auto">
              <table className="w-full text-sm">
                <thead className="bg-gray-900/50">
                  <tr>
                    <th className="px-3 py-2 text-left font-medium text-gray-400 w-16">Page</th>
                    <th className="px-3 py-2 text-left font-medium text-gray-400 w-16">Frame</th>
                    <th className="px-3 py-2 text-left font-medium text-gray-400 w-16">Valid</th>
                    <th className="px-3 py-2 text-left font-medium text-gray-400 w-16">Modified</th>
                    <th className="px-3 py-2 text-left font-medium text-gray-400 w-16">Referenced</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-gray-700">
                  {table.entries.map(entry => (
                    <tr key={entry.page} className={entry.valid ? '' : 'bg-gray-900/50'}>
                      <td className="px-3 py-2 font-mono text-gray-300">{entry.page}</td>
                      <td className="px-3 py-2 font-mono text-gray-300">
                        {entry.frame !== null ? entry.frame : <span className="text-gray-500">—</span>}
                      </td>
                      <td className="px-3 py-2">
                        <span className={`inline-flex items-center justify-center w-6 h-6 rounded text-xs font-medium ${
                          entry.valid ? 'bg-green-900/30 text-green-400' : 'bg-red-900/30 text-red-400'
                        }`}>
                          {entry.valid ? '1' : '0'}
                        </span>
                      </td>
                      <td className="px-3 py-2">
                        <span className={`inline-flex items-center justify-center w-6 h-6 rounded text-xs font-medium ${
                          entry.modified ? 'bg-yellow-900/30 text-yellow-400' : 'bg-gray-700 text-gray-500'
                        }`}>
                          {entry.modified ? '1' : '0'}
                        </span>
                      </td>
                      <td className="px-3 py-2">
                        <span className={`inline-flex items-center justify-center w-6 h-6 rounded text-xs font-medium ${
                          entry.referenced ? 'bg-blue-900/30 text-blue-400' : 'bg-gray-700 text-gray-500'
                        }`}>
                          {entry.referenced ? '1' : '0'}
                        </span>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}