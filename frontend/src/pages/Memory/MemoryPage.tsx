// Memory Page - Memory Grid + Page Table per docs/13_GUI_Architecture.md section 20
import { useEffect, useState } from 'react';
import { MemoryGrid } from '../../components/MemoryGrid/MemoryGrid';
import { PageTableComponent } from '../../components/MemoryGrid/PageTable';
import { api } from '../../services/api';
import type { MemoryFrame, PageTable } from '../../types/os';

export function MemoryPage() {
  const [frames, setFrames] = useState<MemoryFrame[]>([]);
  const [pageTables, setPageTables] = useState<PageTable[]>([]);
  const [selectedPid, setSelectedPid] = useState<number | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [view, setView] = useState<'frames' | 'tables'>('frames');

  const fetchMemory = async () => {
    try {
      const res = await api.getMemory();
      if (res.success && res.data) {
        setFrames(res.data.frames || []);
      } else {
        setError(res.error || 'Failed to fetch memory state');
      }
    } catch (e) {
      setError('Network error fetching memory state');
    } finally {
      setLoading(false);
    }
  };

  const fetchPageTables = async () => {
    try {
      const res = await api.getPageTables();
      if (res.success && res.data) {
        setPageTables(res.data);
        if (res.data.length > 0 && selectedPid === null) {
          setSelectedPid(res.data[0].processId);
        }
      }
    } catch (e) {
      // Silent fail for page tables
    }
  };

  useEffect(() => {
    fetchMemory();
    fetchPageTables();
    const interval = setInterval(() => {
      fetchMemory();
      fetchPageTables();
    }, 2000);
    return () => clearInterval(interval);
  }, []);

  if (loading) {
    return (
      <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center">
        <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-green-500 mx-auto"></div>
        <p className="text-gray-400 mt-4">Loading memory state...</p>
      </div>
    );
  }

  if (error) {
    return (
      <div className="bg-gray-800 border border-red-900 rounded-lg p-6">
        <h2 className="text-xl font-medium text-red-400 mb-2">Error</h2>
        <p className="text-gray-300">{error}</p>
        <button 
          onClick={fetchMemory}
          className="mt-4 px-4 py-2 bg-green-600 hover:bg-green-700 rounded text-white"
        >
          Retry
        </button>
      </div>
    );
  }

  const usedFrames = frames.filter(f => f.owner !== 'Free').length;
  const totalFrames = frames.length;

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg overflow-hidden">
      <div className="p-4 border-b border-gray-700 flex items-center justify-between">
        <h2 className="text-xl font-medium text-gray-100">Memory</h2>
        <div className="flex items-center gap-4">
          <div className="flex gap-2">
            <button
              onClick={() => setView('frames')}
              className={`px-3 py-1 rounded text-sm ${
                view === 'frames' ? 'bg-green-600 text-white' : 'bg-gray-700 text-gray-300 hover:bg-gray-600'
              }`}
            >
              Frame Grid
            </button>
            <button
              onClick={() => setView('tables')}
              className={`px-3 py-1 rounded text-sm ${
                view === 'tables' ? 'bg-green-600 text-white' : 'bg-gray-700 text-gray-300 hover:bg-gray-600'
              }`}
            >
              Page Tables
            </button>
          </div>
          <div className="flex items-center gap-2 text-sm text-gray-400">
            <span>Usage: {totalFrames > 0 ? ((usedFrames / totalFrames) * 100).toFixed(1) : 0}%</span>
          </div>
        </div>
      </div>
      {view === 'frames' && <MemoryGrid frames={frames} />}
      {view === 'tables' && (
        <div className="p-4">
          <PageTableComponent pageTables={pageTables} />
        </div>
      )}
    </div>
  );
}