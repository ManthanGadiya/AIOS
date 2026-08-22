// StatCard Component - System Statistics per docs/13_GUI_Architecture.md section 16
import { STAT_ICONS } from '../../pages/Dashboard/statIcons';

export interface StatCardConfig {
  key: string;
  title: string;
  iconName: keyof typeof STAT_ICONS;
  bgColor: string;
}

interface StatCardProps {
  title: string;
  value: string | number;
  icon: React.ReactNode;
  bgColor: string;
}

export function StatCard({ title, value, icon, bgColor }: StatCardProps) {
  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg p-3 hover:border-gray-600 transition-colors">
      <div className="flex items-start justify-between">
        <div>
          <p className="text-[11px] text-gray-400 uppercase tracking-wide mb-1">{title}</p>
          <p className="text-xl font-bold font-mono text-gray-100">{value}</p>
        </div>
        <div className={`p-2 rounded-lg ${bgColor}`}>
          {icon}
        </div>
      </div>
    </div>
  );
}

// Pre-configured stat cards per the dashboard spec (docs/13 section 16)
export const STAT_CARD_CONFIG: StatCardConfig[] = [
  { key: 'Server', title: 'Total Processes', iconName: 'server', bgColor: 'bg-blue-900/30' },
  { key: 'Bot', title: 'AI Agents', iconName: 'bot', bgColor: 'bg-purple-900/30' },
  { key: 'Cpu', title: 'CPU Usage', iconName: 'cpu', bgColor: 'bg-orange-900/30' },
  { key: 'HardDrive', title: 'Memory Usage', iconName: 'memory', bgColor: 'bg-green-900/30' },
  { key: 'AlertTriangle', title: 'Page Faults', iconName: 'fault', bgColor: 'bg-red-900/30' },
  { key: 'Zap', title: 'Interrupts', iconName: 'bolt', bgColor: 'bg-yellow-900/30' },
];
