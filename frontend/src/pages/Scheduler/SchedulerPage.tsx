// Scheduler Page - Scheduler Panel per docs/13_GUI_Architecture.md section 24
import { useEffect, useState } from 'react';
import { SchedulerPanel } from '../../components/SchedulerPanel/SchedulerPanel';
import { api } from '../../services/api';
import type { SchedulerState } from '../../types/os';

export function SchedulerPage() {
  const [scheduler, setScheduler] = useState<SchedulerState | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchScheduler = async () => {
    try {
      const res = await api.getScheduler();
      if (res.success && res.data) {
        setScheduler(res.data);
      } else {
        setError(res.error || 'Failed to fetch scheduler state');
      }
    } catch (e) {
      setError('Network error fetching scheduler state');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchScheduler();
    const interval = setInterval(fetchScheduler, 1000);
    return () => clearInterval(interval);
  }, []);

  if (loading) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center">
        <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-green-500 mx-auto"></div>
        <p className="text-gray-400 mt-4">Loading scheduler state...</p>
      </div>
    );
  }

  if (error) {
    return (
      <div className="bg-gray-800 border border-red-900 rounded-lg p-6">
        <h2 className="text-xl font-medium text-red-400 mb-2">Error</h2>
        <p className="text-gray-300">{error}</p>
        <button 
          onClick={fetchScheduler}
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
        <h2 className="text-xl font-medium text-gray-100">Scheduler</h2>
      </div>
      <SchedulerPanel scheduler={scheduler} />
    </div>
  );
}