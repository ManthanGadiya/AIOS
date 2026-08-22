// AIOS Frontend Main App
// Per docs/13_GUI_Architecture.md - overall layout with TopBar, Sidebar, Main Content
import { useState, useEffect } from 'react';
import { TopBar } from './components/Header/TopBar';
import { Sidebar } from './components/Sidebar/Sidebar';
import { Dashboard } from './pages/Dashboard/Dashboard';
import type { SchedulingPolicy, SimulationControl } from './types/os';
import { api } from './services/api';
import { ws } from './services/api';
import './index.css';

function App() {
  const [activePage, setActivePage] = useState('dashboard');
  const [currentPolicy, setCurrentPolicy] = useState<SchedulingPolicy>('FCFS');
  const [clockSpeed, setClockSpeed] = useState(1);
  const [isRunning, setIsRunning] = useState(false);
  const [connected, setConnected] = useState(false);
  const [cycle, setCycle] = useState(0);
  const sidebarOpen = true;

  // Poll the simulation clock so the TopBar cycle indicator stays live and
  // the UI can show honestly when the OS engine is unreachable (docs/13
  // section 65: never pretend the simulation is still receiving updates).
  useEffect(() => {
    let cancelled = false;
    const tick = async () => {
      try {
        const res = await api.getStatistics();
        if (!cancelled) {
          setConnected(res.success);
          if (res.success && res.data) setCycle(res.data.cycle ?? 0);
        }
      } catch {
        if (!cancelled) setConnected(false);
      }
    };
    tick();
    const id = setInterval(tick, 1000);
    return () => { cancelled = true; clearInterval(id); };
  }, []);

  const handleCommand = async (cmd: SimulationControl) => {
    const res = await api.sendCommand(cmd);
    // Only reflect state changes the engine actually accepted.
    if (!res.success) return;
    if (cmd.command === 'START_SIMULATION') setIsRunning(true);
    else if (cmd.command === 'PAUSE_SIMULATION') setIsRunning(false);
    else if (cmd.command === 'STOP_SIMULATION') setIsRunning(false);
    else if (cmd.command === 'RESET_SIMULATION') setIsRunning(false);
  };

  const handlePolicyChange = async (policy: SchedulingPolicy) => {
    setCurrentPolicy(policy);
    await api.changeScheduler(policy);
  };

  const handleClockSpeedChange = async (speed: number) => {
    setClockSpeed(speed);
    // TODO: Send clock speed command to backend
  };

  // Initialize WebSocket connection
  useEffect(() => {
    ws.connect().catch(console.error);
    return () => ws.disconnect();
  }, []);

  return (
    <div className="min-h-screen bg-gray-950 text-gray-100 flex flex-col">
      {/* Top Bar */}
      <TopBar
        onCommand={handleCommand}
        currentPolicy={currentPolicy}
        onPolicyChange={handlePolicyChange}
        clockSpeed={clockSpeed}
        onClockSpeedChange={handleClockSpeedChange}
        isRunning={isRunning && connected}
        connected={connected}
        cycle={cycle}
      />

      {/* Main Layout */}
      <div className="flex-1 flex overflow-hidden">
        {/* Sidebar */}
        {sidebarOpen && (
          <Sidebar
            activePage={activePage}
            onPageChange={setActivePage}
            currentPolicy={currentPolicy}
            onPolicyChange={handlePolicyChange}
          />
        )}

        {/* Main Content */}
        <main className={`flex-1 overflow-auto transition-all duration-200 ${sidebarOpen ? 'ml-0' : 'ml-0'}`}>
          <div className="p-4 lg:p-6">
            {activePage === 'dashboard' && <Dashboard connected={connected} />}
            {activePage !== 'dashboard' && (
              <div className="bg-gray-800 border border-gray-700 rounded-lg p-8 text-center">
                <h2 className="text-xl font-medium text-gray-100 mb-2 capitalize">{activePage}</h2>
                <p className="text-gray-500">Page under construction</p>
              </div>
            )}
          </div>
        </main>
      </div>
    </div>
  );
}

export default App;