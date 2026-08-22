// TopBar Component - Simulation controls per docs/13_GUI_Architecture.md sections 7-13
import { Play, Pause, Square, RotateCcw, Cpu, Zap } from 'lucide-react';
import { useState } from 'react';
import type { SimulationControl, SchedulingPolicy } from '../../types/os';

interface TopBarProps {
  onCommand: (cmd: SimulationControl) => void;
  currentPolicy: SchedulingPolicy;
  onPolicyChange: (policy: SchedulingPolicy) => void;
  clockSpeed: number;
  onClockSpeedChange: (speed: number) => void;
  isRunning: boolean;
  connected?: boolean;
  cycle?: number;
}

const POLICIES: { value: SchedulingPolicy; label: string }[] = [
  { value: 'FCFS', label: 'FCFS' },
  { value: 'ROUND_ROBIN', label: 'Round Robin' },
  { value: 'PRIORITY', label: 'Priority' },
  { value: 'AI_ADAPTIVE', label: 'AI Adaptive' },
];

const CLOCK_SPEEDS = [0.5, 1, 2, 5];

export function TopBar({ 
  onCommand, 
  currentPolicy, 
  onPolicyChange, 
  clockSpeed, 
  onClockSpeedChange,
  isRunning,
  connected = false,
  cycle = 0,
}: TopBarProps) {
  const [policyOpen, setPolicyOpen] = useState(false);

  return (
    <header className="bg-gray-900 border-b border-gray-700 px-4 py-3 flex items-center justify-between">
      <div className="flex items-center gap-4">
        <h1 className="text-xl font-mono font-bold text-gray-100">AIOS SIMULATOR</h1>
        <span
          className={`flex items-center gap-1.5 px-2 py-1 rounded text-xs font-medium ${
            connected ? 'bg-green-900/30 text-green-400' : 'bg-red-900/30 text-red-400'
          }`}
          title={connected ? 'OS Engine reachable' : 'OS Engine unreachable - start aios_server.exe'}
        >
          <span className={`w-2 h-2 rounded-full ${connected ? 'bg-green-400 animate-pulse' : 'bg-red-400'}`} />
          {connected ? 'Engine: Connected' : 'Engine: Disconnected'}
        </span>
        <div className="flex items-center gap-2">
          <button
            onClick={() => onCommand({ command: 'START_SIMULATION' })}
            disabled={isRunning}
            className="px-3 py-1.5 bg-green-600 hover:bg-green-700 disabled:opacity-50 disabled:cursor-not-allowed text-white text-sm font-medium rounded flex items-center gap-1.5 transition-colors"
            title="Start Simulation"
          >
            <Play className="w-4 h-4" />
            <span>Start</span>
          </button>
          <button
            onClick={() => onCommand({ command: 'PAUSE_SIMULATION' })}
            disabled={!isRunning}
            className="px-3 py-1.5 bg-yellow-600 hover:bg-yellow-700 disabled:opacity-50 disabled:cursor-not-allowed text-white text-sm font-medium rounded flex items-center gap-1.5 transition-colors"
            title="Pause Simulation"
          >
            <Pause className="w-4 h-4" />
            <span>Pause</span>
          </button>
          <button
            onClick={() => onCommand({ command: 'STOP_SIMULATION' })}
            className="px-3 py-1.5 bg-red-600 hover:bg-red-700 text-white text-sm font-medium rounded flex items-center gap-1.5 transition-colors"
            title="Stop Simulation"
          >
            <Square className="w-4 h-4" />
            <span>Stop</span>
          </button>
          <button
            onClick={() => onCommand({ command: 'RESET_SIMULATION' })}
            className="px-3 py-1.5 bg-gray-600 hover:bg-gray-700 text-white text-sm font-medium rounded flex items-center gap-1.5 transition-colors"
            title="Reset Simulation"
          >
            <RotateCcw className="w-4 h-4" />
            <span>Reset</span>
          </button>
        </div>
      </div>

      <div className="flex items-center gap-6">
        <div className="relative">
          <label className="text-xs text-gray-400 mr-2">Scheduler:</label>
          <div className="relative">
            <button
              onClick={() => setPolicyOpen(!policyOpen)}
              className="px-3 py-1.5 bg-gray-800 border border-gray-600 rounded text-white text-sm font-medium flex items-center gap-2 hover:border-gray-500 transition-colors"
            >
              <Cpu className="w-4 h-4 text-gray-400" />
              <span>{currentPolicy}</span>
              <svg className="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 9l-7 7-7-7" />
              </svg>
            </button>
            {policyOpen && (
              <div className="absolute top-full left-0 mt-1 bg-gray-800 border border-gray-600 rounded shadow-lg z-10 min-w-[140px]">
                {POLICIES.map(p => (
                  <button
                    key={p.value}
                    onClick={() => { onPolicyChange(p.value); setPolicyOpen(false); }}
                    className={`w-full px-3 py-2 text-left text-sm transition-colors ${currentPolicy === p.value ? 'bg-gray-700 text-green-400' : 'text-gray-300 hover:bg-gray-700'}`}
                  >
                    {p.label}
                  </button>
                ))}
              </div>
            )}
          </div>
        </div>

        <div>
          <label className="text-xs text-gray-400 mr-2">Speed:</label>
          <select
            value={clockSpeed}
            onChange={(e) => onClockSpeedChange(Number(e.target.value))}
            className="px-3 py-1.5 bg-gray-800 border border-gray-600 rounded text-white text-sm font-medium focus:outline-none focus:ring-2 focus:ring-green-500"
          >
            {CLOCK_SPEEDS.map(s => (
              <option key={s} value={s}>{s}x</option>
            ))}
          </select>
        </div>

        <div className="flex items-center gap-2 px-3 py-1.5 bg-gray-800 border border-gray-600 rounded">
          <Zap className={`w-4 h-4 ${isRunning ? 'text-yellow-400 animate-pulse' : 'text-gray-500'}`} />
          <span className="text-sm font-mono text-gray-300">Cycle: {cycle}</span>
        </div>
      </div>
    </header>
  );
}