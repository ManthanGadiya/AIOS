// Settings Page - Configuration per docs/13_GUI_Architecture.md section 27
import { useState } from 'react';
import { api } from '../../services/api';
import type { SchedulingPolicy } from '../../types/os';

export function SettingsPage() {
  const [policy, setPolicy] = useState<SchedulingPolicy>('FCFS');
  const [clockSpeed, setClockSpeed] = useState(1);
  const [quantum, setQuantum] = useState(4);
  const [saving, setSaving] = useState(false);
  const [message, setMessage] = useState<{type: 'success' | 'error', text: string} | null>(null);

  const handlePolicyChange = async (newPolicy: SchedulingPolicy) => {
    setPolicy(newPolicy);
    setSaving(true);
    setMessage(null);
    try {
      const res = await api.changeScheduler(newPolicy);
      if (res.success) {
        setMessage({type: 'success', text: `Scheduler changed to ${newPolicy}`});
      } else {
        setMessage({type: 'error', text: res.error || 'Failed to change scheduler'});
      }
    } catch {
      setMessage({type: 'error', text: 'Network error'});
    } finally {
      setSaving(false);
    }
  };

  const handleClockSpeedChange = async (speed: number) => {
    setClockSpeed(speed);
    setSaving(true);
    setMessage(null);
    try {
      const res = await api.setClockSpeed(speed);
      if (res.success) {
        setMessage({type: 'success', text: `Clock speed set to ${speed}x`});
      } else {
        setMessage({type: 'error', text: res.error || 'Failed to set clock speed'});
      }
    } catch {
      setMessage({type: 'error', text: 'Network error'});
    } finally {
      setSaving(false);
    }
  };

  const handleQuantumChange = async (q: number) => {
    setQuantum(q);
    setSaving(true);
    setMessage(null);
    try {
      const res = await api.setQuantum(q);
      if (res.success) {
        setMessage({type: 'success', text: `Time quantum set to ${q} cycles`});
      } else {
        setMessage({type: 'error', text: res.error || 'Failed to set quantum'});
      }
    } catch {
      setMessage({type: 'error', text: 'Network error'});
    } finally {
      setSaving(false);
    }
  };

  const handleReset = async () => {
    setSaving(true);
    setMessage(null);
    try {
      const res = await api.reset();
      if (res.success) {
        setMessage({type: 'success', text: 'Simulation reset successfully'});
      } else {
        setMessage({type: 'error', text: res.error || 'Failed to reset'});
      }
    } catch {
      setMessage({type: 'error', text: 'Network error'});
    } finally {
      setSaving(false);
    }
  };

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg overflow-hidden">
      <div className="p-4 border-b border-gray-700">
        <h2 className="text-xl font-medium text-gray-100">Settings</h2>
      </div>
      <div className="p-6 space-y-8">
        {/* Scheduler Policy */}
        <div className="bg-gray-700/50 rounded-lg p-6">
          <h3 className="text-lg font-medium text-gray-100 mb-4">Scheduler Policy</h3>
          <div className="grid grid-cols-2 md:grid-cols-4 gap-3">
            {(['FCFS', 'ROUND_ROBIN', 'PRIORITY', 'AI_ADAPTIVE'] as SchedulingPolicy[]).map(p => (
              <button
                key={p}
                onClick={() => handlePolicyChange(p)}
                disabled={saving}
                className={`px-4 py-3 rounded-lg text-center font-medium transition-all ${
                  policy === p
                    ? 'bg-green-600 text-white border border-green-500'
                    : 'bg-gray-800 text-gray-300 hover:bg-gray-700 border border-gray-600'
                } disabled:opacity-50`}
              >
                {p}
              </button>
            ))}
          </div>
        </div>

        {/* Clock Speed */}
        <div className="bg-gray-700/50 rounded-lg p-6">
          <h3 className="text-lg font-medium text-gray-100 mb-4">Clock Speed</h3>
          <div className="flex items-center gap-4">
            <input
              type="range"
              min="1"
              max="10"
              value={clockSpeed}
              onChange={(e) => handleClockSpeedChange(parseInt(e.target.value))}
              disabled={saving}
              className="flex-1 h-2 bg-gray-700 rounded-lg appearance-none accent-green-500"
            />
            <span className="text-lg font-mono text-green-400 w-16 text-right">{clockSpeed}x</span>
          </div>
          <p className="text-sm text-gray-400 mt-2">Simulation cycles per second (1-10)</p>
        </div>

        {/* Time Quantum */}
        <div className="bg-gray-700/50 rounded-lg p-6">
          <h3 className="text-lg font-medium text-gray-100 mb-4">Time Quantum (Round Robin)</h3>
          <div className="flex items-center gap-4">
            <input
              type="range"
              min="1"
              max="20"
              value={quantum}
              onChange={(e) => handleQuantumChange(parseInt(e.target.value))}
              disabled={saving}
              className="flex-1 h-2 bg-gray-700 rounded-lg appearance-none accent-green-500"
            />
            <span className="text-lg font-mono text-green-400 w-20 text-right">{quantum} cycles</span>
          </div>
          <p className="text-sm text-gray-400 mt-2">Cycles before preemption in Round Robin mode</p>
        </div>

        {/* Danger Zone */}
        <div className="bg-red-900/20 border border-red-800 rounded-lg p-6">
          <h3 className="text-lg font-medium text-red-400 mb-4">Danger Zone</h3>
          <p className="text-gray-300 mb-4">These actions are destructive and cannot be undone.</p>
          <button
            onClick={handleReset}
            disabled={saving}
            className="px-6 py-3 bg-red-600 hover:bg-red-700 text-white rounded-lg font-medium transition-colors disabled:opacity-50"
          >
            {saving ? 'Resetting...' : 'Reset Simulation'}
          </button>
        </div>

        {/* Status Message */}
        {message && (
          <div className={`p-4 rounded-lg ${message.type === 'success' ? 'bg-green-900/30 border border-green-700' : 'bg-red-900/30 border border-red-700'}`}>
            <p className={`text-${message.type === 'success' ? 'green' : 'red'}-300`}>{message.text}</p>
          </div>
        )}
      </div>
    </div>
  );
}