// Dashboard Page - Main view per docs/13_GUI_Architecture.md sections 15, 59
import { useState, useEffect } from 'react';
import { StatCard, STAT_CARD_CONFIG } from '../../components/StatCard/StatCard';
import { ProcessTable } from '../../components/ProcessTable/ProcessTable';
import { CPUPanel } from '../../components/CPUPanel/CPUPanel';
import { MemoryGrid } from '../../components/MemoryGrid/MemoryGrid';
import { PageTableComponent } from '../../components/MemoryGrid/PageTable';
import { ReadyQueue } from '../../components/ReadyQueue/ReadyQueue';
import { EventLog } from '../../components/EventLog/EventLog';
import { InterruptPanel } from '../../components/InterruptPanel/InterruptPanel';
import { api } from '../../services/api';
import type { SystemStatistics, Process, CPUState, Agent, MemoryFrame, PageTable, SchedulerState, InterruptEvent, EventLogEntry } from '../../types/os';

// Icon mapping for stat cards
const STAT_ICONS: Record<string, React.ReactNode> = {
  Server: (
    <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 12h14M5 12a2 2 0 01-2-2V6a2 2 0 012-2h14a2 2 0 012 2v4a2 2 0 01-2 2M5 12a2 2 0 00-2 2v4a2 2 0 002 2h14a2 2 0 002-2v-4a2 2 0 00-2-2m-2-4h.01M17 16h.01" />
    </svg>
  ),
  Bot: (
    <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9.75 17L9 20l-1 1h8l-1-1-.75-3M3 13h18M5 17h14a2 2 0 002-2V5a2 2 0 00-2-2H5a2 2 0 00-2 2v10a2 2 0 002 2z" />
    </svg>
  ),
  Cpu: (
    <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z" />
    </svg>
  ),
  HardDrive: (
    <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M20 7l-8-4-8 4m16 0l-8 4m8-4v10l-8 4m0-10L4 7m8 4v10M4 7v10l8 4" />
    </svg>
  ),
  AlertTriangle: (
    <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
    </svg>
  ),
  Zap: (
    <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M13 10V3L4 14h7v7l9-11h-7z" />
    </svg>
  ),
};

export function Dashboard() {
  const [statistics, setStatistics] = useState<SystemStatistics | null>(null);
  const [processes, setProcesses] = useState<Process[]>([]);
  const [agents, setAgents] = useState<Agent[]>([]);
  const [cpu, setCpu] = useState<CPUState | null>(null);
  const [memoryFrames, setMemoryFrames] = useState<MemoryFrame[]>([]);
  const [pageTables, setPageTables] = useState<PageTable[]>([]);
  const [scheduler, setScheduler] = useState<SchedulerState | null>(null);
  const [interrupts, setInterrupts] = useState<InterruptEvent[]>([]);
  const [events, setEvents] = useState<EventLogEntry[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchAll = async () => {
    try {
      setError(null);
      const [
        statsRes,
        procsRes,
        agentsRes,
        cpuRes,
        memRes,
        schedRes,
        intRes,
        evtRes,
      ] = await Promise.all([
        api.getStatistics(),
        api.getProcesses(),
        api.getAgents(),
        api.getCPU(),
        api.getMemory(),
        api.getScheduler(),
        api.getInterrupts(),
        api.getEventLog(),
      ]);

      if (statsRes.success) setStatistics(statsRes.data!);
      if (procsRes.success) setProcesses(procsRes.data!);
      if (agentsRes.success) setAgents(agentsRes.data!);
      if (cpuRes.success) setCpu(cpuRes.data!);
      if (memRes.success) {
        setMemoryFrames(memRes.data!.frames);
        setPageTables(memRes.data!.pageTables);
      }
      if (schedRes.success) setScheduler(schedRes.data!);
      if (intRes.success) setInterrupts(intRes.data!);
      if (evtRes.success) setEvents(evtRes.data!);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to fetch dashboard data');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchAll();
    const interval = setInterval(fetchAll, 1000); // Poll every second
    return () => clearInterval(interval);
  }, []);

  if (loading) {
    return (
      <div className="flex items-center justify-center h-[60vh]">
        <div className="animate-spin rounded-full h-12 w-12 border-4 border-gray-600 border-t-green-500" />
      </div>
    );
  }

  if (error) {
    return (
      <div className="bg-red-900/30 border border-red-700 rounded-lg p-4 text-red-300">
        Error loading dashboard: {error}
        <button onClick={fetchAll} className="ml-4 px-3 py-1 bg-red-600 hover:bg-red-700 rounded text-sm">
          Retry
        </button>
      </div>
    );
  }

  const allProcesses = [...processes, ...agents];

  return (
    <div className="space-y-4">
      {/* Stat Cards Row */}
      <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-4">
        {STAT_CARD_CONFIG.map(config => {
          const getValue = () => {
            switch (config.key) {
              case 'Server': return statistics?.totalProcesses ?? 0;
              case 'Bot': return statistics?.aiAgents ?? 0;
              case 'Cpu': return statistics?.cpuUsage ?? 0;
              case 'HardDrive': return statistics?.memoryUsage ?? 0;
              case 'AlertTriangle': return statistics?.pageFaults ?? 0;
              case 'Zap': return statistics?.interrupts ?? 0;
              default: return 0;
            }
          };
          return (
            <StatCard
              key={config.key}
              title={config.title}
              value={getValue()}
              icon={STAT_ICONS[config.key]}
              bgColor={config.bgColor}
            />
          );
        })}
      </div>

      {/* Main Grid - per docs/13 section 59 layout */}
      <div className="grid grid-cols-1 lg:grid-cols-12 gap-4">
        {/* Left Column - Process List + AI Agents */}
        <div className="lg:col-span-6 space-y-4">
          <ProcessTable processes={allProcesses} runningPid={scheduler?.currentProcess ?? null} />
          
          {/* AI Agents - simplified for now */}
          {(agents.length > 0 || processes.some(p => p.type === 'AI_AGENT')) && (
            <div className="bg-gray-800 border border-gray-700 rounded-lg">
              <div className="px-4 py-3 border-b border-gray-700 bg-gray-900 rounded-t-lg">
                <h3 className="font-medium text-gray-100 flex items-center gap-2">
                  <svg className="w-5 h-5 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9.75 17L9 20l-1 1h8l-1-1-.75-3M3 13h18M5 17h14a2 2 0 002-2V5a2 2 0 00-2-2H5a2 2 0 00-2 2v10a2 2 0 002 2z" />
                  </svg>
                  AI Agents
                </h3>
              </div>
              <div className="p-4">
                {agents.length === 0 && processes.filter(p => p.type === 'AI_AGENT').length === 0 ? (
                  <p className="text-gray-500 text-center py-4">No AI agents</p>
                ) : (
                  <div className="space-y-2">
                    {[...agents, ...processes.filter(p => p.type === 'AI_AGENT')].map(agent => (
                      <div key={agent.pid} className="bg-gray-900 border border-gray-700 rounded p-3 flex items-center justify-between">
                        <div className="flex items-center gap-3">
                          <div className="w-8 h-8 rounded-full bg-purple-500/30 flex items-center justify-center">
                            <svg className="w-5 h-5 text-purple-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9.75 17L9 20l-1 1h8l-1-1-.75-3M3 13h18M5 17h14a2 2 0 002-2V5a2 2 0 00-2-2H5a2 2 0 00-2 2v10a2 2 0 002 2z" />
                            </svg>
                          </div>
                          <div>
                            <p className="font-medium text-gray-100">{agent.name || `Agent ${agent.agentId || agent.pid}`}</p>
                            <p className="text-xs text-gray-400">Task: {agent.task || 'Unknown'}</p>
                          </div>
                        </div>
                        <div className="text-right">
                          <p className="text-sm font-mono text-gray-300">Tokens: {agent.tokenUsage?.used ?? 0}/{agent.tokenUsage?.budget ?? 0}</p>
                          <p className="text-xs text-gray-500">PID: {agent.pid}</p>
                        </div>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            </div>
          )}
        </div>

        {/* Right Column - CPU + Memory */}
        <div className="lg:col-span-6 space-y-4">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <CPUPanel cpu={cpu} />
            <MemoryGrid frames={memoryFrames} />
          </div>
          
          <PageTableComponent pageTables={pageTables} />
        </div>

        {/* Bottom Row - Ready Queue + Interrupts + Event Log + IPC */}
        <div className="lg:col-span-12 grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
          <ReadyQueue 
            readyQueue={scheduler?.readyQueue ?? []} 
            processes={allProcesses} 
            runningPid={scheduler?.currentProcess ?? null} 
          />
          <InterruptPanel interrupts={interrupts} />
          <EventLog events={events} maxEvents={50} />
          <div className="bg-gray-800 border border-gray-700 rounded-lg p-4">
            <h3 className="font-medium text-gray-100 mb-3 flex items-center gap-2">
              <svg className="w-5 h-5 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8 12h.01M12 12h.01M16 12h.01M21 12c0 4.418-4.03 8-9 8a9.863 9.863 0 01-4.255-.949L3 20l1.395-3.72C3.512 15.042 3 13.574 3 12c0-4.418 4.03-8 9-8s9 3.582 9 8z" />
              </svg>
              IPC Messages
            </h3>
            <p className="text-gray-500 text-sm">No IPC messages</p>
          </div>
        </div>
      </div>
    </div>
  );
}