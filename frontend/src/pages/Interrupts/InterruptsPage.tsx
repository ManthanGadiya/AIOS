// Interrupts Page - Interrupt Panel per docs/13_GUI_Architecture.md section 25
import { useEffect, useState } from 'react';
import { InterruptPanel } from '../../components/InterruptPanel/InterruptPanel';
import { api } from '../../services/api';
import type { InterruptEvent } from '../../types/os';

export function InterruptsPage() {
  const [interrupts, setInterrupts] = useState<InterruptEvent[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchInterrupts = async () => {
    try {
      const res = await api.getInterrupts();
      if (res.success && res.data) {
        setInterrupts(res.data);
      } else {
        setError(res.error || 'Failed to fetch interrupt state');
      }
    } catch (e) {
      setError('Network error fetching interrupt state');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchInterrupts();
    const interval = setInterval(fetchInterrupts, 1000);
    return () => clearInterval(interval);
  }, []);

  if (loading) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center">
        <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-green-500 mx-auto"></div>
        <p className="text-gray-400 mt-4">Loading interrupts...</p>
      </div>
    );
  }

  if (error) {
    return (
      <div className="bg-gray-800 border border-red-900 rounded-lg p-6">
        <h2 className="text-xl font-medium text-red-400 mb-2">Error</h2>
        <p className="text-gray-300">{error}</p>
        <button 
          onClick={fetchInterrupts}
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
        <h2 className="text-xl font-medium text-gray-100">Interrupts</h2>
      </div>
      <InterruptPanel interrupts={interrupts} />
    </div>
  );
}