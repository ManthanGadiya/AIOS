import { useEffect, useState } from 'react';
import { api } from '../../services/api';
import type { IpcMessage } from '../../types/os';

interface IPCPanelProps {
  autoRefresh?: boolean;
  refreshInterval?: number;
}

export function IPCPanel({ autoRefresh = true, refreshInterval = 2000 }: IPCPanelProps) {
  const [messages, setMessages] = useState<IpcMessage[]>([]);
  const [sending, setSending] = useState(false);
  const [receiving, setReceiving] = useState(false);
  const [sendForm, setSendForm] = useState({
    senderPid: 4,
    receiverPid: 5,
    type: 'TASK_REQUEST',
    payload: '',
  });
  const [receivePid, setReceivePid] = useState(5);
  const [lastResult, setLastResult] = useState<string | null>(null);

  const fetchMessages = async () => {
    try {
      const response = await api.getIPC();
      if (response.success && response.data) {
        setMessages(response.data.messages || []);
      }
    } catch (error) {
      console.error('Failed to fetch IPC messages:', error);
    }
  };

  const handleSend = async (e: React.FormEvent) => {
    e.preventDefault();
    setSending(true);
    setLastResult(null);
    
    const request = {
      senderPid: sendForm.senderPid,
      receiverPid: sendForm.receiverPid,
      type: sendForm.type,
      payload: sendForm.payload,
    };

    try {
      const response = await api.sendIpcMessage(request);
      if (response.success) {
        setLastResult(`Message sent successfully (ID: ${response.data?.messageId})`);
        setSendForm(prev => ({ ...prev, payload: '' }));
        fetchMessages();
      } else {
        setLastResult(`Send failed: ${response.error}`);
      }
    } catch (error) {
      setLastResult(`Send error: ${error instanceof Error ? error.message : 'Unknown error'}`);
    } finally {
      setSending(false);
    }
  };

  const handleReceive = async (e: React.FormEvent) => {
    e.preventDefault();
    setReceiving(true);
    setLastResult(null);

    try {
      const response = await api.receiveIpcMessage({ receiverPid: receivePid, blocking: false });
      if (response.success && response.data?.message) {
        setLastResult(`Received message: ${response.data.message.type} from PID ${response.data.message.senderPid}`);
        fetchMessages();
      } else {
        setLastResult('No message available (queue empty)');
      }
    } catch (error) {
      setLastResult(`Receive error: ${error instanceof Error ? error.message : 'Unknown error'}`);
    } finally {
      setReceiving(false);
    }
  };

  useEffect(() => {
    fetchMessages();
    if (autoRefresh) {
      const interval = setInterval(fetchMessages, refreshInterval);
      return () => clearInterval(interval);
    }
  }, [autoRefresh, refreshInterval]);

  const formatMessageType = (type: string) => {
    const colors: Record<string, string> = {
      'TASK_REQUEST': 'bg-blue-100 text-blue-800',
      'TASK_RESULT': 'bg-green-100 text-green-800',
      'RESEARCH_RESULT': 'bg-purple-100 text-purple-800',
      'DATA': 'bg-gray-100 text-gray-800',
    };
    return colors[type] || 'bg-gray-100 text-gray-800';
  };

  return (
    <div className="bg-gray-800 rounded-lg p-4 h-full flex flex-col overflow-y-auto">
      <h3 className="text-lg font-semibold text-white mb-4 flex items-center gap-2 flex-shrink-0">
        <svg className="w-5 h-5 text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8 12h.01M12 12h.01M16 12h.01M21 12c0 4.418-4.03 8-9 8a9.863 9.863 0 01-4.255-.949L3 20l1.395-3.72C3.512 15.042 3 13.574 3 12c0-4.418 4.03-8 9-8s9 3.582 9 8z" />
        </svg>
        IPC Message Queues
      </h3>

      {/* Send Message Form */}
      <div className="mb-4 p-3 bg-gray-700 rounded border border-gray-600 flex-shrink-0">
        <h4 className="text-sm font-medium text-gray-300 mb-2">Send Message</h4>
        <form onSubmit={handleSend} className="space-y-2">
          <div className="grid grid-cols-2 gap-2">
            <div>
              <label className="text-xs text-gray-400 block mb-1">Sender PID</label>
              <input
                type="number"
                value={sendForm.senderPid}
                onChange={e => setSendForm(prev => ({ ...prev, senderPid: parseInt(e.target.value) }))}
                className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm focus:outline-none focus:ring-1 focus:ring-blue-500"
              />
            </div>
            <div>
              <label className="text-xs text-gray-400 block mb-1">Receiver PID</label>
              <input
                type="number"
                value={sendForm.receiverPid}
                onChange={e => setSendForm(prev => ({ ...prev, receiverPid: parseInt(e.target.value) }))}
                className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm focus:outline-none focus:ring-1 focus:ring-blue-500"
              />
            </div>
          </div>
          <div className="grid grid-cols-2 gap-2">
            <div>
              <label className="text-xs text-gray-400 block mb-1">Message Type</label>
              <select
                value={sendForm.type}
                onChange={e => setSendForm(prev => ({ ...prev, type: e.target.value }))}
                className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm focus:outline-none focus:ring-1 focus:ring-blue-500"
              >
                <option value="TASK_REQUEST">TASK_REQUEST</option>
                <option value="TASK_RESULT">TASK_RESULT</option>
                <option value="RESEARCH_RESULT">RESEARCH_RESULT</option>
                <option value="DATA">DATA</option>
              </select>
            </div>
            <div>
              <label className="text-xs text-gray-400 block mb-1">Payload</label>
              <input
                type="text"
                value={sendForm.payload}
                onChange={e => setSendForm(prev => ({ ...prev, payload: e.target.value }))}
                className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm focus:outline-none focus:ring-1 focus:ring-blue-500"
                placeholder="Message payload..."
              />
            </div>
          </div>
          <button
            type="submit"
            disabled={sending}
            className="w-full px-3 py-2 bg-blue-600 hover:bg-blue-700 disabled:bg-blue-800 text-white text-sm rounded transition-colors"
          >
            {sending ? 'Sending...' : 'Send Message'}
          </button>
        </form>
      </div>

      {/* Receive Message */}
      <div className="mb-4 p-3 bg-gray-700 rounded border border-gray-600 flex-shrink-0">
        <h4 className="text-sm font-medium text-gray-300 mb-2">Receive Message</h4>
        <form onSubmit={handleReceive} className="flex gap-2">
          <div className="flex-1">
            <label className="text-xs text-gray-400 block mb-1">Receiver PID</label>
            <input
              type="number"
              value={receivePid}
              onChange={e => setReceivePid(parseInt(e.target.value))}
              className="w-full px-2 py-1 bg-gray-800 border border-gray-600 rounded text-white text-sm focus:outline-none focus:ring-1 focus:ring-blue-500"
            />
          </div>
          <button
            type="submit"
            disabled={receiving}
            className="px-3 py-2 bg-green-600 hover:bg-green-700 disabled:bg-green-800 text-white text-sm rounded transition-colors self-end"
          >
            {receiving ? 'Receiving...' : 'Receive (Non-blocking)'}
          </button>
        </form>
      </div>

      {/* Result Message */}
      {lastResult && (
        <div className="mb-4 p-2 bg-gray-700 rounded border border-gray-600 text-xs text-gray-300 flex-shrink-0">
          {lastResult}
        </div>
      )}

      {/* Messages List */}
      <div className="flex-1 overflow-y-auto">
        <h4 className="text-sm font-medium text-gray-300 mb-2">Messages ({messages.length})</h4>
        {messages.length === 0 ? (
          <p className="text-gray-500 text-sm text-center py-4">No messages in queues</p>
        ) : (
          <div className="space-y-2 max-h-64 overflow-y-auto">
            {messages.map((msg) => (
              <div
                key={msg.id}
                className="p-2 bg-gray-700 rounded border border-gray-600"
              >
                <div className="flex items-center gap-2 mb-1">
                  <span className={`text-xs px-1.5 py-0.5 rounded ${formatMessageType(msg.type)}`}>
                    {msg.type}
                  </span>
                  <span className="text-xs text-gray-400">ID: {msg.id}</span>
                  <span className="text-xs text-gray-400">Ch: {msg.channelId || 'N/A'}</span>
                </div>
                <div className="flex gap-2 text-xs text-gray-300">
                  <span>From: P{msg.senderPid}</span>
                  <span>To: P{msg.receiverPid}</span>
                  <span>@{msg.timestamp}</span>
                </div>
                <div className="mt-1 text-xs text-gray-400 truncate">
                  {msg.payload}
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}

export default IPCPanel;