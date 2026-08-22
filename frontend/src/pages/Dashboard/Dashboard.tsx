// Dashboard Page - Main view per docs/13_GUI_Architecture.md sections 15, 58-60
// All panels use fixed heights with internal scrollbars so the whole system
// state fits on screen with minimal page scrolling (docs/13 section 60).
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
import { STAT_ICONS } from './statIcons';
import type { SystemStatistics, Process, CPUState, MemoryFrame, PageTable, SchedulerState, InterruptEvent, EventLogEntry } from '../../types/os';

interface DashboardProps {
  connected?: boolean;
}

export function Dashboard({ connected = true }: DashboardProps) {
  const [statistics, setStatistics] = useState<SystemStatistics | null>(null);
  const [processes, setProcesses] = useState<Process[]>([]);
  const [cpu, setCpu] = useState<CPUState | null>(null);
  const [memoryFrames, setMemoryFrames] = useState<MemoryFrame[]>([]);
  const [pageTables, setPageTables] = useState<PageTable[]>([]);
  const [scheduler, setScheduler] = useState<SchedulerState | null>(null);
  const [interrupts, setInterrupts] = useState<InterruptEvent[]>([]);
  const [events, setEvents] = useState<EventLogEntry[]>([]);

  const fetchAll = async () => {
    const [
      statsRes,
      procsRes,
      cpuRes,
      memRes,
      schedRes,
      intRes,
      evtRes,
    ] = await Promise.all([
      api.getStatistics(),
      api.getProcesses(),
      api.getCPU(),
      api.getMemory(),
      api.getScheduler(),
      api.getInterrupts(),
      api.getEventLog(),
    ]);

    // Only update a panel when its fetch succeeded; failed requests leave the
    // last known values in place while the banner reports the disconnection.
    if (statsRes.success) setStatistics(statsRes.data!);
    if (procsRes.success) setProcesses(procsRes.data!);
    if (cpuRes.success) setCpu(cpuRes.data!);
    if (memRes.success) {
      setMemoryFrames(memRes.data!.frames);
      setPageTables(memRes.data!.pageTables);
    }
    if (schedRes.success) setScheduler(schedRes.data!);
    if (intRes.success) setInterrupts(intRes.data!);
    if (evtRes.success) setEvents(evtRes.data!);
  };

  useEffect(() => {
    fetchAll();
    const interval = setInterval(fetchAll, 1000);
    return () => clearInterval(interval);
  }, []);

  const allProcesses = [...processes];

  return (
    <div className="space-y-4">
      {/* Backend disconnection notice (docs/13 section 65) */}
      {!connected && (
        <div className="bg-red-900/30 border border-red-700 rounded-lg px-4 py-3 text-red-300 text-sm flex items-center gap-2">
          <span className="w-2 h-2 rounded-full bg-red-400 animate-pulse" />
          OS Engine: Disconnected — showing last known state. Start the engine with
          <code className="px-1.5 py-0.5 bg-gray-900 rounded font-mono text-xs">backend\build\aios_server.exe 8081</code>
        </div>
      )}

      {/* Level 1 - Overall system (docs/13 section 58) */}
      <div className="grid grid-cols-3 lg:grid-cols-6 gap-3">
        {STAT_CARD_CONFIG.map(config => {
          const getValue = (): string => {
            switch (config.key) {
              case 'Server': return `${statistics?.totalProcesses ?? 0}`;
              case 'Bot': return `${statistics?.aiAgents ?? 0}`;
              case 'Cpu': return `${statistics?.cpuUsage ?? 0}%`;
              case 'HardDrive': return `${statistics?.memoryUsage ?? 0}%`;
              case 'AlertTriangle': return `${statistics?.pageFaults ?? 0}`;
              case 'Zap': return `${statistics?.interrupts ?? 0}`;
              default: return '0';
            }
          };
          return (
            <StatCard
              key={config.key}
              title={config.title}
              value={getValue()}
              icon={STAT_ICONS[config.iconName]}
              bgColor={config.bgColor}
            />
          );
        })}
      </div>

      {/* Level 2 - Process list spans full width, agents marked by Type column */}
      <ProcessTable processes={allProcesses} runningPid={scheduler?.currentProcess ?? null} />

      {/* CPU / Memory / Paging row */}
      <div className="grid grid-cols-1 xl:grid-cols-3 gap-4">
        <CPUPanel cpu={cpu} />
        <MemoryGrid frames={memoryFrames} />
        <PageTableComponent pageTables={pageTables} />
      </div>

      {/* Activity row: ready queue, interrupts, event log, IPC (IPC is a later milestone) */}
      <div className="grid grid-cols-1 xl:grid-cols-4 gap-4">
        <div className="h-64 flex flex-col">
          <ReadyQueue
            readyQueue={scheduler?.readyQueue ?? []}
            processes={allProcesses}
            runningPid={scheduler?.currentProcess ?? null}
          />
        </div>
        <div className="h-64 flex flex-col">
          <InterruptPanel interrupts={interrupts} />
        </div>
        <div className="h-64 flex flex-col">
          <EventLog events={events} maxEvents={60} />
        </div>
        <div className="h-64 bg-gray-800 border border-gray-700 rounded-lg p-4 flex flex-col">
          <h3 className="font-medium text-gray-100 mb-2 flex items-center gap-2 text-sm">
            <svg className="w-4 h-4 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8 12h.01M12 12h.01M16 12h.01M21 12c0 4.418-4.03 8-9 8a9.863 9.863 0 01-4.255-.949L3 20l1.395-3.72C3.512 15.042 3 13.574 3 12c0-4.418 4.03-8 9-8s9 3.582 9 8z" />
            </svg>
            IPC Messages
          </h3>
          <div className="flex-1 overflow-y-auto text-sm text-gray-500">IPC manager arrives with Week 6</div>
        </div>
      </div>
    </div>
  );
}
