#include <aios/core/types.hpp>

#include <algorithm>
#include <cctype>
#include <map>
#include <string>

namespace aios {

std::string opcodeToString(Opcode opcode) {
    switch (opcode) {
        case Opcode::LOAD:    return "LOAD";
        case Opcode::STORE:   return "STORE";
        case Opcode::ADD:     return "ADD";
        case Opcode::SUB:     return "SUB";
        case Opcode::MUL:     return "MUL";
        case Opcode::DIV:     return "DIV";
        case Opcode::JMP:     return "JMP";
        case Opcode::JZ:      return "JZ";
        case Opcode::READ:    return "READ";
        case Opcode::WRITE:   return "WRITE";
        case Opcode::SYSCALL: return "SYSCALL";
        case Opcode::HALT:    return "HALT";
        case Opcode::INVALID: return "INVALID";
    }
    return "UNKNOWN";
}

std::string Flags::toString() const {
    return "Z=" + std::to_string(zero ? 1 : 0) +
           " N=" + std::to_string(negative ? 1 : 0) +
           " C=" + std::to_string(carry ? 1 : 0) +
           " V=" + std::to_string(overflow ? 1 : 0) +
           " E=" + std::to_string(error ? 1 : 0);
}

bool Instruction::parse(const std::string& text, Instruction& out) {
    // Trim surrounding whitespace.
    const size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return false;
    }
    const size_t last = text.find_last_not_of(" \t\r\n");
    std::string line = text.substr(first, last - first + 1);

    // Split mnemonic and (optional) operand at the first whitespace.
    const size_t sp = line.find_first_of(" \t");
    std::string mnemonic = (sp == std::string::npos) ? line : line.substr(0, sp);
    std::string operandText = (sp == std::string::npos) ? "" : line.substr(sp + 1);

    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

    static const std::map<std::string, Opcode> table = {
        {"LOAD", Opcode::LOAD},   {"STORE", Opcode::STORE},
        {"ADD", Opcode::ADD},     {"SUB", Opcode::SUB},
        {"MUL", Opcode::MUL},     {"DIV", Opcode::DIV},
        {"JMP", Opcode::JMP},     {"JZ", Opcode::JZ},
        {"READ", Opcode::READ},   {"WRITE", Opcode::WRITE},
        {"SYSCALL", Opcode::SYSCALL}, {"HALT", Opcode::HALT},
    };
    const auto it = table.find(mnemonic);
    if (it == table.end()) {
        return false;
    }
    const Opcode op = it->second;

    if (op == Opcode::HALT) {
        if (!operandText.empty()) {
            return false; // HALT takes no operand.
        }
        out = Instruction{op, 0};
        return true;
    }

    const size_t oFirst = operandText.find_first_not_of(" \t");
    const size_t oLast = operandText.find_last_not_of(" \t");
    if (oFirst == std::string::npos) {
        return false; // Operand required for every instruction except HALT.
    }
    const std::string operand = operandText.substr(oFirst, oLast - oFirst + 1);
    try {
        size_t consumed = 0;
        const long value = std::stol(operand, &consumed);
        if (consumed != operand.size()) {
            return false;
        }
        out = Instruction{op, static_cast<int32_t>(value)};
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string Instruction::toString() const {
    if (isEmpty()) {
        return "INVALID";
    }
    if (opcode == Opcode::HALT) {
        return "HALT";
    }
    return opcodeToString(opcode) + " " + std::to_string(operand);
}

std::string CpuRegisters::toString() const {
    return "PC=" + std::to_string(pc) +
           " ACC=" + std::to_string(acc) +
           " MAR=" + std::to_string(mar) +
           " MBR=" + std::to_string(mbr) +
           " IR=" + ir.toString() +
           " " + flags.toString();
}

std::string processStateToString(ProcessState state) {
    switch (state) {
        case ProcessState::NEW:        return "NEW";
        case ProcessState::READY:      return "READY";
        case ProcessState::RUNNING:    return "RUNNING";
        case ProcessState::WAITING:    return "WAITING";
        case ProcessState::TERMINATED: return "TERMINATED";
        case ProcessState::FAILED:     return "FAILED";
    }
    return "UNKNOWN";
}

std::string processTypeToString(ProcessType type) {
    switch (type) {
        case ProcessType::NORMAL:   return "NORMAL";
        case ProcessType::AI_AGENT: return "AI_AGENT";
    }
    return "UNKNOWN";
}

std::string cpuStateToString(CpuState state) {
    switch (state) {
        case CpuState::IDLE:       return "IDLE";
        case CpuState::RUNNING:    return "RUNNING";
        case CpuState::INTERRUPTED: return "INTERRUPTED";
        case CpuState::HALTED:     return "HALTED";
    }
    return "UNKNOWN";
}

std::string interruptTypeToString(InterruptType type) {
    switch (type) {
        case InterruptType::TIMER:        return "TIMER";
        case InterruptType::SYSTEM_CALL:  return "SYSTEM_CALL";
        case InterruptType::IO_COMPLETE:  return "IO_COMPLETE";
        case InterruptType::ERR:        return "ERR";
        case InterruptType::PAGE_FAULT:   return "PAGE_FAULT";
    }
    return "UNKNOWN";
}

std::string eventTypeToString(EventType type) {
    switch (type) {
        case EventType::PROCESS_CREATED:     return "PROCESS_CREATED";
        case EventType::PROCESS_STATE_CHANGED: return "PROCESS_STATE_CHANGED";
        case EventType::CONTEXT_SWITCH:      return "CONTEXT_SWITCH";
        case EventType::PAGE_FAULT:          return "PAGE_FAULT";
        case EventType::INTERRUPT:           return "INTERRUPT";
        case EventType::IPC_MESSAGE:         return "IPC_MESSAGE";
        case EventType::IO_REQUEST:          return "IO_REQUEST";
        case EventType::IO_COMPLETE:         return "IO_COMPLETE";
        case EventType::PROCESS_TERMINATED:  return "PROCESS_TERMINATED";
        case EventType::INSTRUCTION_FETCH:   return "INSTRUCTION_FETCH";
        case EventType::INSTRUCTION_EXECUTE: return "INSTRUCTION_EXECUTE";
        case EventType::REGISTER_UPDATE:     return "REGISTER_UPDATE";
        case EventType::SYSCALL:             return "SYSCALL";
        case EventType::HALT:                return "HALT";
        case EventType::CPU_ERROR:           return "CPU_ERROR";
        case EventType::PAGE_ALLOCATED:      return "PAGE_ALLOCATED";
        case EventType::PAGE_ACCESSED:       return "PAGE_ACCESSED";
        case EventType::FRAME_ALLOCATED:     return "FRAME_ALLOCATED";
        case EventType::PAGE_REPLACED:       return "PAGE_REPLACED";
        case EventType::PAGE_LOADED:         return "PAGE_LOADED";
        case EventType::PAGE_SWAPPED_OUT:    return "PAGE_SWAPPED_OUT";
        case EventType::PAGE_SWAPPED_IN:     return "PAGE_SWAPPED_IN";
        case EventType::INVALID_MEMORY_ACCESS: return "INVALID_MEMORY_ACCESS";
        case EventType::SCHEDULER_STARTED:     return "SCHEDULER_STARTED";
        case EventType::PROCESS_SELECTED:      return "PROCESS_SELECTED";
        case EventType::PROCESS_PREEMPTED:     return "PROCESS_PREEMPTED";
        case EventType::TIME_QUANTUM_EXPIRED:  return "TIME_QUANTUM_EXPIRED";
        // Synchronization
        case EventType::MUTEX_CREATED:       return "MUTEX_CREATED";
        case EventType::MUTEX_LOCKED:        return "MUTEX_LOCKED";
        case EventType::MUTEX_UNLOCKED:      return "MUTEX_UNLOCKED";
        case EventType::PROCESS_BLOCKED_ON_MUTEX: return "PROCESS_BLOCKED_ON_MUTEX";
        case EventType::PROCESS_WAKEUP:      return "PROCESS_WAKEUP";
        case EventType::SEMAPHORE_CREATED:   return "SEMAPHORE_CREATED";
        case EventType::SEMAPHORE_WAIT:      return "SEMAPHORE_WAIT";
        case EventType::SEMAPHORE_SIGNAL:    return "SEMAPHORE_SIGNAL";
        case EventType::DEADLOCK_DETECTED:   return "DEADLOCK_DETECTED";
        case EventType::SYNCHRONIZATION_ERROR: return "SYNCHRONIZATION_ERROR";
        // IPC
        case EventType::IPC_CHANNEL_CREATED:  return "IPC_CHANNEL_CREATED";
        case EventType::IPC_SEND:             return "IPC_SEND";
        case EventType::IPC_RECEIVE:          return "IPC_RECEIVE";
        case EventType::IPC_MESSAGE_QUEUED:   return "IPC_MESSAGE_QUEUED";
        case EventType::IPC_MESSAGE_DELIVERED: return "IPC_MESSAGE_DELIVERED";
        case EventType::IPC_QUEUE_FULL:       return "IPC_QUEUE_FULL";
        case EventType::IPC_PERMISSION_DENIED: return "IPC_PERMISSION_DENIED";
        case EventType::IPC_ERROR:            return "IPC_ERROR";
        // I/O
        case EventType::IO_ERROR:             return "IO_ERROR";
        case EventType::IO_STARTED:           return "IO_STARTED";
        case EventType::IO_BLOCKED:           return "IO_BLOCKED";
        // AI Agent
        case EventType::AGENT_CREATED:        return "AGENT_CREATED";
        case EventType::AGENT_STARTED:        return "AGENT_STARTED";
        case EventType::AGENT_CHECKPOINTED:   return "AGENT_CHECKPOINTED";
        case EventType::AGENT_TOOL_REQUEST:   return "AGENT_TOOL_REQUEST";
        case EventType::AGENT_TOOL_DENIED:    return "AGENT_TOOL_DENIED";
        case EventType::AGENT_FAILURE:        return "AGENT_FAILURE";
        case EventType::AGENT_RECOVERY_STARTED: return "AGENT_RECOVERY_STARTED";
        case EventType::AGENT_RECOVERED:      return "AGENT_RECOVERED";
        case EventType::AGENT_TERMINATED:     return "AGENT_TERMINATED";
    }
    return "UNKNOWN";
}

std::string agentTypeToString(AgentType type) {
    switch (type) {
        case AgentType::RESEARCH: return "RESEARCH";
        case AgentType::CODING:   return "CODING";
    }
    return "UNKNOWN";
}

std::string agentStateToString(AgentState state) {
    switch (state) {
        case AgentState::ACTIVE:       return "ACTIVE";
        case AgentState::WAITING:      return "WAITING";
        case AgentState::CHECKPOINTED: return "CHECKPOINTED";
        case AgentState::FAILED:       return "FAILED";
        case AgentState::RECOVERED:    return "RECOVERED";
    }
    return "UNKNOWN";
}

std::string toolPermissionToString(ToolPermission perm) {
    switch (perm) {
        case ToolPermission::READ_DATA:  return "READ_DATA";
        case ToolPermission::SEARCH:     return "SEARCH";
        case ToolPermission::WRITE_CODE: return "WRITE_CODE";
        case ToolPermission::EXECUTE:    return "EXECUTE";
    }
    return "UNKNOWN";
}

std::string resourceTypeToString(ResourceType type) {
    switch (type) {
        case ResourceType::CPU:     return "CPU";
        case ResourceType::MEMORY:  return "MEMORY";
        case ResourceType::TOKEN:   return "TOKEN";
        case ResourceType::IO:      return "IO";
    }
    return "UNKNOWN";
}

std::string failureTypeToString(FailureType type) {
    switch (type) {
        case FailureType::RESOURCE_LIMIT:         return "RESOURCE_LIMIT";
        case FailureType::SIMULATED_CRASH:        return "SIMULATED_CRASH";
        case FailureType::EXECUTION_ERROR:        return "EXECUTION_ERROR";
        case FailureType::TOOL_PERMISSION_ERROR:  return "TOOL_PERMISSION_ERROR";
        case FailureType::MEMORY_ERROR:           return "MEMORY_ERROR";
    }
    return "UNKNOWN";
}

} // namespace aios