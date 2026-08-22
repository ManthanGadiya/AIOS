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
  const [cycle, setCycle] = useState(0);
  const sidebarOpen = true;

  // Poll the simulation clock so the TopBar cycle indicator stays live.
  useEffect(() => {
    let cancelled = false;
    const tick = async () => {
      try {
        const res = await api.getStatistics();
        if (!cancelled && res.success && res.data) {
          setCycle(res.data.cycle ?? 0);
        }
      } catch {
        // Backend offline; keep last known value.
      }
    };
    const id = setInterval(tick, 1000);
    return () => { cancelled = true; clearInterval(id); };
  }, []);

  const handleCommand = async (cmd: SimulationControl) => {
    try {
      await api.sendCommand(cmd);
      if (cmd.command === 'START_SIMULATION') setIsRunning(true);
      else if (cmd.command === 'STOP_SIMULATION') setIsRunning(false);
      else if (cmd.command === 'RESET_SIMULATION') setIsRunning(false);
    } catch (error) {
      console.error('Command failed:', error);
    }
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
        isRunning={isRunning}
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
            {activePage === 'dashboard' && <Dashboard />}
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