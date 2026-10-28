// CPU Page - CPU Panel per docs/13_GUI_Architecture.md section 19
import { useEffect, useState } from 'react';
import { CPUPanel } from '../../components/CPUPanel/CPUPanel';
import { api } from '../../services/api';
import type { CPUState } from '../../types/os';

export function CPUPage() {
  const [cpu, setCpu] = useState<CPUState | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchCPU = async () => {
    try {
      const res = await api.getCPU();
      if (res.success && res.data) {
        setCpu(res.data);
      } else {
        setError(res.error || 'Failed to fetch CPU state');
      }
    } catch (e) {
      setError('Network error fetching CPU state');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchCPU();
    const interval = setInterval(fetchCPU, 1000);
    return () => clearInterval(interval);
  }, []);

  if (loading) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center">
        <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-green-500 mx-auto"></div>
        <p className="text-gray-400 mt-4">Loading CPU state...</p>
      </div>
    );
  }

  if (error || !cpu) {
    return (
      <div className="bg-gray-800 border border-red-900 rounded-lg p-6">
        <h2 className="text-xl font-medium text-red-400 mb-2">Error</h2>
        <p className="text-gray-300">{error || 'No CPU data available'}</p>
        <button 
          onClick={fetchCPU}
          className="mt-4 px-4 py-2 bg-green-600 hover:bg-green-700 rounded text-white"
        >
          Retry
        </button>
      </div>
    );
  }

  const isRunning = cpu.pc > 0; // simple heuristic - PC > 0 means it's executing

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg overflow-hidden">
      <div className="p-4 border-b border-gray-700 flex items-center justify-between">
        <h2 className="text-xl font-medium text-gray-100">CPU</h2>
        <span className={`px-2 py-1 rounded text-xs font-medium ${
          isRunning ? 'bg-green-900 text-green-300' : 'bg-gray-700 text-gray-300'
        }`}>
          {isRunning ? 'RUNNING' : 'IDLE'}
        </span>
      </div>
      <CPUPanel cpu={cpu} />
    </div>
  );
}