// StatCard Component - System Statistics per docs/13_GUI_Architecture.md section 16
import { Server, Bot, Cpu, HardDrive, AlertTriangle, Zap } from 'lucide-react';

interface StatCardProps {
  title: string;
  value: string | number;
  icon: React.ReactNode;
  bgColor: string;
  trend?: string;
}

export function StatCard({ title, value, icon, bgColor, trend }: StatCardProps) {
  return (
    <div className={`bg-gray-800 border border-gray-700 rounded-lg p-4 hover:border-gray-600 transition-colors`}>
      <div className="flex items-start justify-between">
        <div>
          <p className="text-xs text-gray-400 uppercase tracking-wide mb-1">{title}</p>
          <p className="text-2xl font-bold font-mono text-gray-100">{value}</p>
          {trend && (
            <p className="text-xs text-green-400 mt-1">{trend}</p>
          )}
        </div>
        <div className={`p-2 rounded-lg ${bgColor}`}>
          {icon}
        </div>
      </div>
    </div>
  );
}

// Pre-configured stat cards per the dashboard spec
export const STAT_CARD_CONFIG = [
  { key: 'Server', title: 'Total Processes', icon: Server, bgColor: 'bg-blue-900/30' },
  { key: 'Bot', title: 'AI Agents', icon: Bot, bgColor: 'bg-purple-900/30' },
  { key: 'Cpu', title: 'CPU Usage', icon: Cpu, bgColor: 'bg-orange-900/30', suffix: '%' },
  { key: 'HardDrive', title: 'Memory Usage', icon: HardDrive, bgColor: 'bg-green-900/30', suffix: '%' },
  { key: 'AlertTriangle', title: 'Page Faults', icon: AlertTriangle, bgColor: 'bg-red-900/30' },
  { key: 'Zap', title: 'Interrupts', icon: Zap, bgColor: 'bg-yellow-900/30' },
] as const;