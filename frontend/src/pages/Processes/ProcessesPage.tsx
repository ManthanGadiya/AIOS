// Processes Page - Process Table per docs/13_GUI_Architecture.md section 17
import { useEffect, useState } from 'react';
import { ProcessTable } from '../../components/ProcessTable/ProcessTable';
import { api } from '../../services/api';
import type { Process } from '../../types/os';

export function ProcessesPage() {
  const [processes, setProcesses] = useState<Process[]>([]);
  const [runningPid, setRunningPid] = useState<number | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchProcesses = async () => {
    try {
      const [procRes, schedRes] = await Promise.all([
        api.getAllProcesses(),
        api.getScheduler(),
      ]);
      if (procRes.success && procRes.data) {
        // Finished processes drop out of the table; the event log keeps the
        // full history (docs/13 section 17).
        setProcesses(
          procRes.data.filter((p) => p.state !== 'TERMINATED')
        );
      } else {
        setError(procRes.error || 'Failed to fetch processes');
      }
      if (schedRes.success && schedRes.data) {
        setRunningPid(schedRes.data.currentProcess);
      }
    } catch (e) {
      setError('Network error fetching processes');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchProcesses();
    const interval = setInterval(fetchProcesses, 2000);
    return () => clearInterval(interval);
  }, []);

  if (loading) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center">
        <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-green-500 mx-auto"></div>
        <p className="text-gray-400 mt-4">Loading processes...</p>
      </div>
    );
  }

  if (error) {
    return (
      <div className="bg-gray-800 border border-red-900 rounded-lg p-6">
        <h2 className="text-xl font-medium text-red-400 mb-2">Error</h2>
        <p className="text-gray-300">{error}</p>
        <button 
          onClick={fetchProcesses}
          className="mt-4 px-4 py-2 bg-green-600 hover:bg-green-700 rounded text-white"
        >
          Retry
        </button>
      </div>
    );
  }

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg overflow-hidden">
      <div className="p-4 border-b border-gray-700">
        <h2 className="text-xl font-medium text-gray-100">Processes</h2>
      </div>
      <ProcessTable processes={processes} runningPid={runningPid} />
    </div>
  );
}