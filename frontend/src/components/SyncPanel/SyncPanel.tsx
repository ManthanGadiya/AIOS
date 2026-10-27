import { useEffect, useState } from 'react';
import { api } from '../../services/api';
import type { Mutex, Semaphore } from '../../types/os';

export function SyncPanel() {
  const [mutexes, setMutexes] = useState<Mutex[]>([]);
  const [semaphores, setSemaphores] = useState<Semaphore[]>([]);
  const [lastResult, setLastResult] = useState<string | null>(null);
  const [lockForm, setLockForm] = useState({ mutexId: 'M1', pid: 4 });
  const [unlockForm, setUnlockForm] = useState({ mutexId: 'M1', pid: 4 });
  const [waitForm, setWaitForm] = useState({ semaphoreId: 'S1', pid: 4 });
  const [signalForm, setSignalForm] = useState({ semaphoreId: 'S1' });

  const fetchState = async () => {
    try {
      const [mutexRes, semRes] = await Promise.all([
        api.getMutexes(),
        api.getSemaphores(),
      ]);
      if (mutexRes.success && mutexRes.data) setMutexes(mutexRes.data);
      if (semRes.success && semRes.data) setSemaphores(semRes.data);
    } catch (error) {
      console.error('Failed to fetch sync state:', error);
    }
  };

  const handleLock = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      const res = await api.lockMutex({ mutexId: lockForm.mutexId, pid: lockForm.pid });
      if (res.success) {
        setLastResult(`Mutex ${lockForm.mutexId} locked by PID ${lockForm.pid}`);
        fetchState();
      } else {
        setLastResult(`Lock failed: PID ${lockForm.pid} blocked on ${lockForm.mutexId}`);
        fetchState();
      }
    } catch (error) {
      setLastResult(`Lock error: ${error instanceof Error ? error.message : 'Unknown error'}`);
    }
  };

  const handleUnlock = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      const res = await api.unlockMutex({ mutexId: unlockForm.mutexId, pid: unlockForm.pid });
      const nextWaiter = res.data?.nextWaiter;
      if (res.success) {
        setLastResult(`Mutex ${unlockForm.mutexId} unlocked by PID ${unlockForm.pid}${nextWaiter !== undefined && nextWaiter !== -1 ? `, next waiter: ${nextWaiter}` : ''}`);
        fetchState();
      } else {
        setLastResult(`Unlock failed`);
      }
    } catch (error) {
      setLastResult(`Unlock error: ${error instanceof Error ? error.message : 'Unknown error'}`);
    }
  };

  const handleWait = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      const res = await api.waitSemaphore({ semaphoreId: waitForm.semaphoreId, pid: waitForm.pid });
      if (res.success) {
        setLastResult(`Semaphore ${waitForm.semaphoreId} acquired by PID ${waitForm.pid}`);
      } else {
        setLastResult(`Semaphore ${waitForm.semaphoreId} unavailable, PID ${waitForm.pid} blocked`);
      }
      fetchState();
    } catch (error) {
      setLastResult(`Wait error: ${error instanceof Error ? error.message : 'Unknown error'}`);
    }
  };

  const handleSignal = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      const res = await api.signalSemaphore({ semaphoreId: signalForm.semaphoreId });
      const nextWaiter = res.data?.nextWaiter;
      if (res.success) {
        setLastResult(`Semaphore ${signalForm.semaphoreId} signaled${nextWaiter !== undefined && nextWaiter !== -1 ? `, waiter ${nextWaiter} woken` : ''}`);
        fetchState();
      } else {
        setLastResult(`Signal failed`);
      }
    } catch (error) {
      setLastResult(`Signal error: ${error instanceof Error ? error.message : 'Unknown error'}`);
    }
  };

  useEffect(() => {
    fetchState();
    const interval = setInterval(fetchState, 2000);
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="bg-gray-800 rounded-lg p-4 h-full flex flex-col">
      <h3 className="text-lg font-semibold text-white mb-4 flex items-center gap-2">
        <svg className="w-5 h-5 text-yellow-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2h10a2 2 0 002-2v-6a2 2 0 00-2-2H8a2 2 0 01-2-2V7a2 2 0 012-2h8a2 2 0 012 2v2" />
        </svg>
        Synchronization
      </h3>

      {/* Mutexes */}
      <div className="mb-6">
        <h4 className="text-sm font-medium text-gray-300 mb-2 flex items-center gap-2">
          <svg className="w-4 h-4 text-yellow-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2h10a2 2 0 002-2v-6a2 2 0 00-2-2H8a2 2 0 01-2-2V7a2 2 0 012-2h8a2 2 0 012 2v2" />
          </svg>
          Mutexes ({mutexes.length})
        </h4>
        {mutexes.length === 0 ? (
          <p className="text-gray-500 text-sm text-center py-2">No mutexes</p>
        ) : (
          <div className="space-y-2 max-h-48 overflow-y-auto">
            {mutexes.map((m) => (
              <div key={m.id} className="p-2 bg-gray-700 rounded border border-gray-600">
                <div className="flex items-center gap-2 mb-1">
                  <span className="font-mono text-sm font-medium">{m.id}</span>
                  <span className={`text-xs px-1.5 py-0.5 rounded ${m.locked ? 'bg-red-100 text-red-800' : 'bg-green-100 text-green-800'}`}>
                    {m.locked ? 'LOCKED' : 'FREE'}
                  </span>
                  {m.locked && (
                    <span className="text-xs text-gray-400">Owner: P{m.ownerPid}</span>
                  )}
                  <span className="text-xs text-gray-400">Waiters: {m.waiterCount}</span>
                </div>
                {m.waiters.length > 0 && (
                  <div className="mt-1 text-xs text-gray-500">
                    Waiting: {m.waiters.map(p => `P${p}`).join(', ')}
                  </div>
                )}
              </div>
            ))}
          </div>
        )}
      </div>

      {/* Semaphores */}
      <div className="mb-6">
        <h4 className="text-sm font-medium text-gray-300 mb-2 flex items-center gap-2">
          <svg className="w-4 h-4 text-green-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
          </svg>
          Semaphores ({semaphores.length})
        </h4>
        {semaphores.length === 0 ? (
          <p className="text-gray-500 text-sm text-center py-2">No semaphores</p>
        ) : (
          <div className="space-y-2 max-h-48 overflow-y-auto">
            {semaphores.map((s) => (
              <div key={s.id} className="p-2 bg-gray-700 rounded border border-gray-600">
                <div className="flex items-center gap-2 mb-1">
                  <span className="font-mono text-sm font-medium">{s.id}</span>
                  <span className="text-sm font-medium">Count: {s.count}</span>
                  <span className="text-xs text-gray-400">Waiters: {s.waiterCount}</span>
                </div>
                {s.waiters.length > 0 && (
                  <div className="text-xs text-gray-500">
                    Waiting: {s.waiters.map(p => `P${p}`).join(', ')}
                  </div>
                )}
              </div>
            ))}
          </div>
        )}
      </div>

      {/* Result */}
      {lastResult && (
        <div className="mb-4 p-2 bg-gray-700 rounded border border-gray-600 text-xs text-gray-300">
          {lastResult}
        </div>
      )}

      {/* Controls */}
      <div className="space-y-4">
        {/* Lock Mutex */}
        <div className="p-3 bg-gray-700 rounded border border-gray-600">
          <h4 className="text-sm font-medium text-gray-300 mb-2">Lock Mutex</h4>
          <form onSubmit={handleLock} className="flex gap-2">
            <div className="flex-1">
              <label className="text-xs text-gray-400 block mb-1">Mutex ID</label>
              <select value={lockForm.mutexId} onChange={e => setLockForm(prev => ({ ...prev, mutexId: e.target.value }))} className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm">
                <option value="M1">M1</option>
                <option value="knowledge_bus">knowledge_bus</option>
              </select>
            </div>
            <div className="w-24">
              <label className="text-xs text-gray-400 block mb-1">PID</label>
              <input type="number" value={lockForm.pid} onChange={e => setLockForm(prev => ({ ...prev, pid: parseInt(e.target.value) }))} className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm" />
            </div>
            <button type="submit" className="px-3 py-2 bg-yellow-600 hover:bg-yellow-700 text-white text-sm rounded transition-colors self-end">Lock</button>
          </form>
        </div>

        {/* Unlock Mutex */}
        <div className="p-3 bg-gray-700 rounded border border-gray-600">
          <h4 className="text-sm font-medium text-gray-300 mb-2">Unlock Mutex</h4>
          <form onSubmit={handleUnlock} className="flex gap-2">
            <div className="flex-1">
              <label className="text-xs text-gray-400 block mb-1">Mutex ID</label>
              <select value={unlockForm.mutexId} onChange={e => setUnlockForm(prev => ({ ...prev, mutexId: e.target.value }))} className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm">
                <option value="M1">M1</option>
                <option value="knowledge_bus">knowledge_bus</option>
              </select>
            </div>
            <div className="w-24">
              <label className="text-xs text-gray-400 block mb-1">PID</label>
              <input type="number" value={unlockForm.pid} onChange={e => setUnlockForm(prev => ({ ...prev, pid: parseInt(e.target.value) }))} className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm" />
            </div>
            <button type="submit" className="px-3 py-2 bg-gray-600 hover:bg-gray-700 text-white text-sm rounded transition-colors self-end">Unlock</button>
          </form>
        </div>

        {/* Wait Semaphore */}
        <div className="p-3 bg-gray-700 rounded border border-gray-600">
          <h4 className="text-sm font-medium text-gray-300 mb-2">Wait Semaphore</h4>
          <form onSubmit={handleWait} className="flex gap-2">
            <div className="flex-1">
              <label className="text-xs text-gray-400 block mb-1">Semaphore ID</label>
              <select value={waitForm.semaphoreId} onChange={e => setWaitForm(prev => ({ ...prev, semaphoreId: e.target.value }))} className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm">
                <option value="S1">S1</option>
              </select>
            </div>
            <div className="w-24">
              <label className="text-xs text-gray-400 block mb-1">PID</label>
              <input type="number" value={waitForm.pid} onChange={e => setWaitForm(prev => ({ ...prev, pid: parseInt(e.target.value) }))} className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm" />
            </div>
            <button type="submit" className="px-3 py-2 bg-green-600 hover:bg-green-700 text-white text-sm rounded transition-colors self-end">Wait</button>
          </form>
        </div>

        {/* Signal Semaphore */}
        <div className="p-3 bg-gray-700 rounded border border-gray-600">
          <h4 className="text-sm font-medium text-gray-300 mb-2">Signal Semaphore</h4>
          <form onSubmit={handleSignal} className="flex gap-2">
            <div className="flex-1">
              <label className="text-xs text-gray-400 block mb-1">Semaphore ID</label>
              <select value={signalForm.semaphoreId} onChange={e => setSignalForm(prev => ({ ...prev, semaphoreId: e.target.value }))} className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm">
                <option value="S1">S1</option>
              </select>
            </div>
            <button type="submit" className="px-3 py-2 bg-green-600 hover:bg-green-700 text-white text-sm rounded transition-colors self-end">Signal</button>
          </form>
        </div>

        {/* Result */}
        {lastResult && (
          <div className="p-2 bg-gray-700 rounded border border-gray-600 text-xs text-gray-300">
            {lastResult}
          </div>
        )}
      </div>
    </div>
  );
}

export default SyncPanel;