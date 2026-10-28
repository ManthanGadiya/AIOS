// IPC Page - IPC Panel per docs/13_GUI_Architecture.md section 23
import { useEffect, useState } from 'react';
import { IPCPanel } from '../../components/IPCPanel/IPCPanel';
import { api } from '../../services/api';

export function IPCPage() {
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchIPC = async () => {
    try {
      const res = await api.getIPC();
      if (!res.success) {
        setError(res.error || 'Failed to fetch IPC state');
      }
    } catch (e) {
      setError('Network error fetching IPC state');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchIPC();
    const interval = setInterval(fetchIPC, 1000);
    return () => clearInterval(interval);
  }, []);

  if (loading) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center">
        <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-green-500 mx-auto"></div>
        <p className="text-gray-400 mt-4">Loading IPC state...</p>
      </div>
    );
  }

  if (error) {
    return (
      <div className="bg-gray-800 border border-red-900 rounded-lg p-6">
        <h2 className="text-xl font-medium text-red-400 mb-2">Error</h2>
        <p className="text-gray-300">{error}</p>
        <button 
          onClick={fetchIPC}
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
        <h2 className="text-xl font-medium text-gray-100">IPC</h2>
      </div>
      <IPCPanel />
    </div>
  );
}