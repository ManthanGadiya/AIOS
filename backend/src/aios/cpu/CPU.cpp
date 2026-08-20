#include <aios/cpu/CPU.hpp>

#include <aios/interrupt/InterruptManager.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/memory/MemoryManager.hpp>

#include <limits>
#include <string>

namespace aios {

void CPU::initialize(int32_t initialPc) {
    regs_ = CpuRegisters{};
    regs_.pc = initialPc;
    state_ = CpuState::IDLE;
    currentPid_ = INVALID_PID;
}

void CPU::loadContext(const CpuRegisters& regs, int pid) {
    regs_ = regs;
    currentPid_ = pid;
    state_ = CpuState::RUNNING;
}

CpuRegisters CPU::saveContext() const {
    return regs_;
}

void CPU::reset() {
    state_ = CpuState::IDLE;
    regs_ = CpuRegisters{};
    currentPid_ = INVALID_PID;
}

void CPU::executeCycle() {
    if (state_ != CpuState::RUNNING) {
        return; // only meaningful while a process context is loaded
    }
    fetch();
    if (state_ != CpuState::RUNNING) {
        return; // fetch halted the CPU or raised a fault for the OS to service
    }
    execute();
}

void CPU::resume() {
    if (state_ == CpuState::INTERRUPTED) {
        state_ = CpuState::RUNNING;
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void CPU::fetch() {
    if (!memory_) {
        raiseError("fetch: memory not attached");
        return;
    }
    if (regs_.pc < 0) {
        raiseError("fetch: program counter out of bounds: " + std::to_string(regs_.pc));
        return;
    }
    regs_.mar = regs_.pc;

    MemoryAccessResult result;
    if (memoryManager_) {
        // Paged mode (docs/07): the fetch address is a logical address.
        result = memoryManager_->accessMemory(currentPid_,
                                              static_cast<uint32_t>(regs_.mar), false, 0);
        if (result.status == MemoryAccessStatus::PAGE_FAULT) {
            // PC is not advanced yet, so the retry re-fetches the same word
            // (docs/07 section 16).
            raisePageFault(result.page, /*rollbackPc=*/false);
            return;
        }
        if (result.status == MemoryAccessStatus::INVALID) {
            raiseInvalidAccess(static_cast<uint32_t>(regs_.mar));
            return;
        }
        regs_.mbr = result.value;
    } else {
        if (regs_.pc >= static_cast<int32_t>(Memory::MEMORY_SIZE)) {
            raiseError("fetch: program counter out of bounds: " + std::to_string(regs_.pc));
            return;
        }
        const auto word = memory_->read(static_cast<uint32_t>(regs_.mar));
        if (!word) {
            raiseError("fetch: read failed at " + std::to_string(regs_.mar));
            return;
        }
        regs_.mbr = *word;
    }
    regs_.ir = Instruction::fromWord(regs_.mbr);
    if (regs_.ir.opcode == Opcode::INVALID) {
        raiseError("fetch: invalid opcode in word " + std::to_string(regs_.mbr));
        return;
    }
    ++regs_.pc; // one word per instruction (docs/04 section 11)
    record(EventType::INSTRUCTION_FETCH,
           "PC=" + std::to_string(regs_.mar) + " IR=" + regs_.ir.toString());
}

void CPU::execute() {
    record(EventType::INSTRUCTION_EXECUTE, regs_.ir.toString());

    switch (regs_.ir.opcode) {
        case Opcode::LOAD: {
            if (memoryManager_) {
                const MemoryAccessResult r = memoryManager_->accessMemory(
                    currentPid_, regs_.ir.operand, false, 0);
                if (r.status == MemoryAccessStatus::PAGE_FAULT) {
                    raisePageFault(r.page, /*rollbackPc=*/true);
                    break;
                }
                if (r.status == MemoryAccessStatus::INVALID) {
                    raiseInvalidAccess(regs_.ir.operand);
                    break;
                }
                regs_.acc = r.value;
            } else {
                const auto value = memory_->read(static_cast<uint32_t>(regs_.ir.operand));
                if (!value) {
                    raiseError("LOAD: address out of bounds: " + std::to_string(regs_.ir.operand));
                    break;
                }
                regs_.acc = *value;
            }
            regs_.flags.zero = (regs_.acc == 0);
            regs_.flags.negative = (regs_.acc < 0);
            regs_.flags.carry = false;
            regs_.flags.overflow = false;
            break;
        }
        case Opcode::STORE: {
            if (memoryManager_) {
                const MemoryAccessResult r = memoryManager_->accessMemory(
                    currentPid_, regs_.ir.operand, true, regs_.acc);
                if (r.status == MemoryAccessStatus::PAGE_FAULT) {
                    raisePageFault(r.page, /*rollbackPc=*/true);
                    break;
                }
                if (r.status == MemoryAccessStatus::INVALID) {
                    raiseInvalidAccess(regs_.ir.operand);
                    break;
                }
            } else {
                if (!memory_->write(static_cast<uint32_t>(regs_.ir.operand), regs_.acc)) {
                    raiseError("STORE: address out of bounds: " + std::to_string(regs_.ir.operand));
                }
            }
            break;
        }
        case Opcode::ADD: {
            const int64_t exact = static_cast<int64_t>(regs_.acc) + regs_.ir.operand;
            regs_.acc = static_cast<int32_t>(exact);
            updateArithmeticFlags(regs_.acc, exact, true);
            break;
        }
        case Opcode::SUB: {
            const int64_t exact = static_cast<int64_t>(regs_.acc) - regs_.ir.operand;
            regs_.acc = static_cast<int32_t>(exact);
            updateArithmeticFlags(regs_.acc, exact, true);
            break;
        }
        case Opcode::MUL: {
            const int64_t exact = static_cast<int64_t>(regs_.acc) * regs_.ir.operand;
            regs_.acc = static_cast<int32_t>(exact);
            updateArithmeticFlags(regs_.acc, exact, false);
            break;
        }
        case Opcode::DIV: {
            if (regs_.ir.operand == 0) {
                raiseError("DIV: division by zero");
                break;
            }
            if (regs_.acc == std::numeric_limits<int32_t>::min() && regs_.ir.operand == -1) {
                raiseError("DIV: integer overflow");
                break;
            }
            regs_.acc /= regs_.ir.operand;
            updateArithmeticFlags(regs_.acc, regs_.acc, false);
            break;
        }
        case Opcode::JMP: {
            if (!isJumpTargetValid(regs_.ir.operand)) {
                raiseError("JMP: target out of bounds: " + std::to_string(regs_.ir.operand));
                break;
            }
            regs_.pc = regs_.ir.operand;
            break;
        }
        case Opcode::JZ: {
            if (!regs_.flags.zero) {
                break; // fall through
            }
            if (!isJumpTargetValid(regs_.ir.operand)) {
                raiseError("JZ: target out of bounds: " + std::to_string(regs_.ir.operand));
                break;
            }
            regs_.pc = regs_.ir.operand;
            break;
        }
        case Opcode::READ: {
            if (!input_) {
                raiseError("READ: no input attached");
                break;
            }
            if (input_->empty()) {
                raiseError("READ: input queue empty");
                break;
            }
            const int32_t value = input_->front();
            input_->pop_front();
            if (memoryManager_) {
                const MemoryAccessResult r = memoryManager_->accessMemory(
                    currentPid_, regs_.ir.operand, true, value);
                if (r.status == MemoryAccessStatus::PAGE_FAULT) {
                    raisePageFault(r.page, /*rollbackPc=*/true);
                    break;
                }
                if (r.status == MemoryAccessStatus::INVALID) {
                    raiseInvalidAccess(regs_.ir.operand);
                    break;
                }
            } else {
                if (!memory_->write(static_cast<uint32_t>(regs_.ir.operand), value)) {
                    raiseError("READ: address out of bounds: " + std::to_string(regs_.ir.operand));
                }
            }
            break;
        }
        case Opcode::WRITE: {
            if (!output_) {
                raiseError("WRITE: no output attached");
                break;
            }
            int32_t value = 0;
            if (memoryManager_) {
                const MemoryAccessResult r = memoryManager_->accessMemory(
                    currentPid_, regs_.ir.operand, false, 0);
                if (r.status == MemoryAccessStatus::PAGE_FAULT) {
                    raisePageFault(r.page, /*rollbackPc=*/true);
                    break;
                }
                if (r.status == MemoryAccessStatus::INVALID) {
                    raiseInvalidAccess(regs_.ir.operand);
                    break;
                }
                value = r.value;
            } else {
                const auto word = memory_->read(static_cast<uint32_t>(regs_.ir.operand));
                if (!word) {
                    raiseError("WRITE: address out of bounds: " + std::to_string(regs_.ir.operand));
                    break;
                }
                value = *word;
            }
            *output_ << value << "\n";
            break;
        }
        case Opcode::SYSCALL: {
            record(EventType::SYSCALL, "SYSCALL " + std::to_string(regs_.ir.operand));
            if (!interruptManager_) {
                raiseError("SYSCALL: interrupt manager not attached");
                break;
            }
            interruptManager_->generateInterrupt(InterruptType::SYSTEM_CALL, currentPid_,
                                                 regs_.ir.operand);
            state_ = CpuState::INTERRUPTED; // context saved by the OS (docs/05)
            break;
        }
        case Opcode::HALT: {
            record(EventType::HALT, "HALT");
            state_ = CpuState::HALTED;
            break;
        }
        case Opcode::INVALID: {
            raiseError("execute: invalid opcode");
            break;
        }
    }
}

void CPU::updateArithmeticFlags(int32_t result, int64_t exact, bool isAddOrSub) {
    regs_.flags.zero = (result == 0);
    regs_.flags.negative = (result < 0);
    regs_.flags.overflow = (exact != static_cast<int64_t>(result));
    // Carry mirrors wraparound: on this flat word machine any 32-bit
    // wraparound is treated as carry out (decision D6, docs/04 section 12).
    regs_.flags.carry = regs_.flags.overflow;
    (void)isAddOrSub; // reserved for unsigned-carry refinement in later weeks
}

void CPU::raiseError(const std::string& detail) {
    record(EventType::CPU_ERROR, detail);
    regs_.flags.error = true;
    state_ = CpuState::HALTED;
}

bool CPU::isJumpTargetValid(int32_t target) const {
    if (target < 0) {
        return false;
    }
    if (memoryManager_) {
        // In paged mode the logical space is defined by the page table; it may
        // be larger than physical RAM, so validate against logical space.
        return memoryManager_->isValidLogicalAddress(
            currentPid_, static_cast<uint32_t>(target));
    }
    return target < static_cast<int32_t>(Memory::MEMORY_SIZE);
}

void CPU::raisePageFault(uint32_t page, bool rollbackPc) {
    if (rollbackPc) {
        regs_.pc = regs_.mar; // retry the faulting instruction (docs/07 section 16)
    }
    record(EventType::PAGE_FAULT,
           "page=" + std::to_string(page) + " retry_pc=" + std::to_string(regs_.pc));
    if (!interruptManager_) {
        raiseError("PAGE_FAULT: interrupt manager not attached");
        return;
    }
    // The interrupt manager loads the page via MemoryManager, then the OS
    // resumes the CPU so the faulting instruction is retried.
    interruptManager_->generateInterrupt(InterruptType::PAGE_FAULT, currentPid_,
                                         static_cast<int32_t>(page));
    state_ = CpuState::INTERRUPTED;
}

void CPU::raiseInvalidAccess(uint32_t logicalAddress) {
    record(EventType::INVALID_MEMORY_ACCESS, "addr=" + std::to_string(logicalAddress));
    if (!interruptManager_) {
        raiseError("INVALID_MEMORY_ACCESS: interrupt manager not attached");
        return;
    }
    // An out-of-bounds access is an unrecoverable error: the ERROR interrupt
    // marks the process FAILED (docs/05 section 4).
    interruptManager_->generateInterrupt(InterruptType::ERROR, currentPid_,
                                         static_cast<int32_t>(logicalAddress));
    state_ = CpuState::INTERRUPTED;
}

void CPU::record(EventType type, const std::string& detail) {
    if (!eventLog_) {
        return;
    }
    eventLog_->record(type, currentPid_, clock_ ? clock_->cycle() : 0, detail);
}

} // namespace aios