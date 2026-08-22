// EventLog Component - Chronological event stream per docs/13_GUI_Architecture.md sections 40-42
import { useState, useMemo } from 'react';
import { Filter, ChevronDown } from 'lucide-react';
import type { EventLogEntry } from '../../types/os';

const CATEGORIES = [
  'ALL', 'PROCESS', 'CPU', 'MEMORY', 'SCHEDULER', 
  'INTERRUPT', 'IPC', 'IO', 'SYNCHRONIZATION', 'AI_AGENT', 'ERROR'
] as const;

const CATEGORY_COLORS: Record<string, string> = {
  PROCESS: 'bg-blue-900/30 text-blue-400',
  CPU: 'bg-orange-900/30 text-orange-400',
  MEMORY: 'bg-green-900/30 text-green-400',
  SCHEDULER: 'bg-purple-900/30 text-purple-400',
  INTERRUPT: 'bg-red-900/30 text-red-400',
  IPC: 'bg-pink-900/30 text-pink-400',
  IO: 'bg-cyan-900/30 text-cyan-400',
  SYNCHRONIZATION: 'bg-amber-900/30 text-amber-400',
  AI_AGENT: 'bg-indigo-900/30 text-indigo-400',
  ERROR: 'bg-red-900/30 text-red-400',
};

interface EventLogProps {
  events: EventLogEntry[];
  maxEvents?: number;
}

export function EventLog({ events, maxEvents = 100 }: EventLogProps) {
  const [filter, setFilter] = useState<'ALL' | typeof CATEGORIES[number]>('ALL');
  const [search, setSearch] = useState('');
  const [expandedId, setExpandedId] = useState<number | null>(null);

  const filteredEvents = useMemo(() => {
    return events
      .filter(e => filter === 'ALL' || e.category === filter)
      .filter(e => search === '' || e.message.toLowerCase().includes(search.toLowerCase()))
      .slice(-maxEvents)
      .reverse(); // Most recent first
  }, [events, filter, search, maxEvents]);

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg flex flex-col h-full">
      <div className="px-4 py-3 border-b border-gray-700 bg-gray-900 rounded-t-lg flex flex-col gap-3">
        <div className="flex items-center justify-between">
          <h3 className="font-medium text-gray-100 flex items-center gap-2">
            <svg className="w-5 h-5 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 5H7a2 2 0 00-2 2v12a2 2 0 002 2h10a2 2 0 002-2V7a2 2 0 00-2-2h-2M9 5a2 2 0 002 2h2a2 2 0 002-2M9 5a2 2 0 012-2h2a2 2 0 012 2m-6 9l2 2 4-4" />
            </svg>
            Event Log
          </h3>
          <span className="text-xs text-gray-400 font-mono">{filteredEvents.length} events</span>
        </div>
        
        <div className="flex flex-wrap gap-2">
          <div className="relative flex-1 min-w-[200px]">
            <Filter className="absolute left-2 top-1/2 -translate-y-1/2 w-4 h-4 text-gray-500" />
            <input
              type="text"
              placeholder="Search events..."
              value={search}
              onChange={e => setSearch(e.target.value)}
              className="w-full pl-9 pr-4 py-1.5 bg-gray-800 border border-gray-600 rounded text-white text-sm placeholder-gray-500 focus:outline-none focus:ring-2 focus:ring-green-500"
            />
          </div>
          
          <div className="relative">
            <button
              onClick={() => setFilter(filter === 'ALL' ? 'PROCESS' : 'ALL')} // Simplified for now
              className="px-3 py-1.5 bg-gray-800 border border-gray-600 rounded text-white text-sm flex items-center gap-1.5 hover:border-gray-500 transition-colors"
            >
              <span className="capitalize">{filter === 'ALL' ? 'All' : filter}</span>
              <ChevronDown className="w-4 h-4" />
            </button>
          </div>
        </div>
      </div>

      <div className="flex-1 overflow-y-auto p-3 space-y-1">
        {filteredEvents.length === 0 ? (
          <div className="text-center text-gray-500 py-8">No events matching filter</div>
        ) : (
          filteredEvents.map(event => (
            <EventLogEntryComponent 
              key={event.id} 
              event={event} 
              isExpanded={expandedId === event.id}
              onToggle={() => setExpandedId(expandedId === event.id ? null : event.id)}
            />
          ))
        )}
      </div>
    </div>
  );
}

function EventLogEntryComponent({ event, isExpanded, onToggle }: { event: EventLogEntry; isExpanded: boolean; onToggle: () => void }) {
  const categoryStyle = CATEGORY_COLORS[event.category] || 'bg-gray-700 text-gray-300';
  
  return (
    <div className="bg-gray-900/50 border border-gray-700 rounded p-3 hover:border-gray-600 transition-colors">
      <div className="flex items-start gap-3" onClick={onToggle}>
        <div className={`px-2 py-0.5 rounded text-xs font-medium ${categoryStyle} flex-shrink-0`}>
          {event.category}
        </div>
        <div className="flex-1 min-w-0">
          <div className="flex items-center gap-2 text-sm">
            <span className="font-mono text-gray-400">{event.timestamp}</span>
            {event.pid && (
              <span className="px-1.5 py-0.5 bg-gray-700 rounded text-xs font-mono text-gray-300">
                PID {event.pid}
              </span>
            )}
          </div>
          <p className="text-gray-200 mt-1 break-all">{event.message}</p>
        </div>
        <svg 
          className={`w-4 h-4 text-gray-500 flex-shrink-0 transition-transform ${isExpanded ? 'rotate-180' : ''}`}
          fill="none" 
          stroke="currentColor" 
          viewBox="0 0 24 24"
        >
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 9l-7 7-7-7" />
        </svg>
      </div>
      
      {isExpanded && (
        <div className="mt-3 pt-3 border-t border-gray-700 bg-gray-900/50 rounded p-3 text-xs">
          <pre className="text-gray-300 font-mono whitespace-pre-wrap break-all">
            {JSON.stringify(event, null, 2)}
          </pre>
        </div>
      )}
    </div>
  );
}