// Sidebar Component - Navigation per docs/13_GUI_Architecture.md section 14
import { 
  LayoutDashboard, Cpu, HardDrive, Server, 
  GitBranch, Activity, Network, Database, 
  TerminalSquare, Settings
} from 'lucide-react';
import type { SchedulingPolicy } from '../../types/os';

interface SidebarProps {
  activePage: string;
  onPageChange: (page: string) => void;
  currentPolicy: SchedulingPolicy;
  onPolicyChange: (policy: SchedulingPolicy) => void;
}

const PAGES = [
  { id: 'dashboard', label: 'Dashboard', icon: LayoutDashboard },
  { id: 'processes', label: 'Processes', icon: Server },
  { id: 'agents', label: 'AI Agents', icon: Network },
  { id: 'cpu', label: 'CPU', icon: Cpu },
  { id: 'memory', label: 'Memory', icon: HardDrive },
  { id: 'io', label: 'I/O Devices', icon: Database },
  { id: 'ipc', label: 'IPC', icon: GitBranch },
  { id: 'scheduler', label: 'Scheduler', icon: Activity },
  { id: 'interrupts', label: 'Interrupts', icon: TerminalSquare },
  { id: 'logs', label: 'Logs', icon: TerminalSquare },
  { id: 'settings', label: 'Settings', icon: Settings },
];

export function Sidebar({ activePage, onPageChange, currentPolicy, onPolicyChange }: SidebarProps) {
  return (
    <aside className="w-64 bg-gray-900 border-r border-gray-700 flex flex-col h-full">
      <nav className="flex-1 p-4 space-y-1 overflow-y-auto">
        {PAGES.map(page => (
          <button
            key={page.id}
            onClick={() => onPageChange(page.id)}
            className={`w-full px-3 py-2.5 rounded-lg text-left transition-all flex items-center gap-3 ${
              activePage === page.id
                ? 'bg-gray-800 text-green-400 border border-gray-600'
                : 'text-gray-300 hover:bg-gray-800 hover:text-white'
            }`}
          >
            <page.icon className="w-5 h-5 flex-shrink-0" />
            <span className="font-medium">{page.label}</span>
          </button>
        ))}
      </nav>
      <div className="p-4 border-t border-gray-700">
        <div className="space-y-3">
          <div>
            <label className="text-xs text-gray-400 block mb-1">Scheduler Policy</label>
            <select
              value={currentPolicy}
              onChange={(e) => onPolicyChange(e.target.value as SchedulingPolicy)}
              className="w-full px-2 py-1.5 bg-gray-800 border border-gray-600 rounded text-white text-sm focus:outline-none focus:ring-2 focus:ring-green-500"
            >
              <option value="FCFS">FCFS</option>
              <option value="ROUND_ROBIN">Round Robin</option>
              <option value="PRIORITY">Priority</option>
              <option value="AI_ADAPTIVE">AI Adaptive</option>
            </select>
          </div>
        </div>
      </div>
    </aside>
  );
}