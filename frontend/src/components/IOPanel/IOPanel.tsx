import { useEffect, useState } from 'react';
import { api } from '../../services/api';
import type { IODevice } from '../../types/os';

interface IOPanelProps {
  autoRefresh?: boolean;
  refreshInterval?: number;
}

export function IOPanel({ autoRefresh = true, refreshInterval = 3000 }: IOPanelProps) {
  const [devices, setDevices] = useState<IODevice[]>([]);

  const fetchDevices = async () => {
    try {
      const response = await api.getIODevices();
      if (response.success && response.data) {
        setDevices(response.data);
      }
    } catch (error) {
      console.error('Failed to fetch I/O devices:', error);
    }
  };

  useEffect(() => {
    fetchDevices();
    if (autoRefresh) {
      const interval = setInterval(fetchDevices, refreshInterval);
      return () => clearInterval(interval);
    }
  }, [autoRefresh, refreshInterval]);

  const getStatusColor = (state: string) => {
    return state === 'BUSY' ? 'bg-red-100 text-red-800' : 'bg-green-100 text-green-800';
  };

  const getStatusIcon = (state: string) => {
    if (state === 'BUSY') {
      return (
        <svg className="w-3 h-3 animate-spin" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4" />
          <path className="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z" />
        </svg>
      );
    }
    return (
      <svg className="w-3 h-3" fill="currentColor" viewBox="0 0 20 20">
        <path fillRule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-9.293a1 1 0 00-1.414-1.414L9 10.586 7.707 9.293a1 1 0 00-1.414 1.414l2 2a1 1 0 001.414 0l4-4z" clipRule="evenodd" />
      </svg>
    );
  };

  return (
    <div className="bg-gray-800 rounded-lg p-4 h-full flex flex-col">
      <h3 className="text-lg font-semibold text-white mb-4 flex items-center gap-2">
        <svg className="w-5 h-5 text-orange-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 4v16h16V4H4zm6 6h.01M16 16h.01M9 12h.01M15 12h.01M12 9h.01M12 15h.01" />
        </svg>
        I/O Devices ({devices.length})
      </h3>

      {devices.length === 0 ? (
        <div className="flex-1 flex flex-col items-center justify-center text-center">
          <svg className="w-12 h-12 text-gray-600 mb-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 4v16h16V4H4zm6 6h.01M16 16h.01M9 12h.01M15 12h.01M12 9h.01M12 15h.01" />
          </svg>
          <p className="text-gray-500">I/O Subsystem not yet implemented (Week 7)</p>
          <p className="text-gray-600 text-sm mt-2">Device abstraction, buffer cache, and spooler coming in Week 7</p>
        </div>
      ) : (
        <div className="flex-1 overflow-y-auto space-y-3">
          {devices.map((device) => (
            <div key={device.name} className="bg-gray-700 rounded-lg border border-gray-600 p-4">
              <div className="flex items-center justify-between mb-3">
                <div className="flex items-center gap-3">
                  <div className="w-10 h-10 bg-orange-600 rounded-full flex items-center justify-center">
                    <svg className="w-6 h-6 text-white" fill="currentColor" viewBox="0 0 24 24">
                      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 4v16h16V4H4zm6 6h.01M16 16h.01M9 12h.01M15 12h.01M12 9h.01M12 15h.01" />
                    </svg>
                  </div>
                  <div>
                    <div className="font-medium text-white">{device.name}</div>
                    <div className="text-xs text-gray-400">Device</div>
                  </div>
                </div>
                <span className={`px-2 py-0.5 text-xs rounded ${getStatusColor(device.state)}`}>
                  {device.state}
                  {getStatusIcon(device.state)}
                </span>
              </div>

              <div className="grid grid-cols-2 gap-4">
                <div>
                  <div className="text-xs text-gray-400">Queue Length</div>
                  <div className="text-sm font-medium text-white">{device.queue.length}</div>
                </div>
                <div>
                  <div className="text-xs text-gray-400">Waiting Processes</div>
                  <div className="text-sm font-medium text-white">
                    {device.queue.length > 0 ? device.queue.join(', ') : 'None'}
                  </div>
                </div>
              </div>

              {device.queue.length > 0 && (
                <div className="mt-3 p-2 bg-gray-800 rounded">
                  <div className="text-xs text-gray-400 mb-1">Waiting Queue:</div>
                  <div className="flex flex-wrap gap-1">
                    {device.queue.map((proc, idx) => (
                      <span key={idx} className="px-2 py-0.5 text-xs bg-gray-600 rounded">
                        {proc}
                      </span>
                    ))}
                  </div>
                </div>
              )}
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

export default IOPanel;