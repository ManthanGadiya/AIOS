#include <aios/cpu/CPU.hpp>

#include <aios/interrupt/InterruptManager.hpp>
#include <aios/memory/Memory.hpp>

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
    if (isHalted()) {
        return; // fetch raised an error
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
    if (regs_.pc < 0 || regs_.pc >= static_cast<int32_t>(Memory::MEMORY_SIZE)) {
        raiseError("fetch: program counter out of bounds: " + std::to_string(regs_.pc));
        return;
    }
    regs_.mar = regs_.pc;
    const auto word = memory_->read(static_cast<uint32_t>(regs_.mar));
    if (!word) {
        raiseError("fetch: read failed at " + std::to_string(regs_.mar));
        return;
    }
    regs_.mbr = *word;
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
            const auto value = memory_->read(static_cast<uint32_t>(regs_.ir.operand));
            if (!value) {
                raiseError("LOAD: address out of bounds: " + std::to_string(regs_.ir.operand));
                break;
            }
            regs_.acc = *value;
            regs_.flags.zero = (regs_.acc == 0);
            regs_.flags.negative = (regs_.acc < 0);
            regs_.flags.carry = false;
            regs_.flags.overflow = false;
            break;
        }
        case Opcode::STORE: {
            if (!memory_->write(static_cast<uint32_t>(regs_.ir.operand), regs_.acc)) {
                raiseError("STORE: address out of bounds: " + std::to_string(regs_.ir.operand));
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
            if (regs_.ir.operand < 0 ||
                regs_.ir.operand >= static_cast<int32_t>(Memory::MEMORY_SIZE)) {
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
            if (regs_.ir.operand < 0 ||
                regs_.ir.operand >= static_cast<int32_t>(Memory::MEMORY_SIZE)) {
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
            if (!memory_->write(static_cast<uint32_t>(regs_.ir.operand), value)) {
                raiseError("READ: address out of bounds: " + std::to_string(regs_.ir.operand));
            }
            break;
        }
        case Opcode::WRITE: {
            if (!output_) {
                raiseError("WRITE: no output attached");
                break;
            }
            const auto value = memory_->read(static_cast<uint32_t>(regs_.ir.operand));
            if (!value) {
                raiseError("WRITE: address out of bounds: " + std::to_string(regs_.ir.operand));
                break;
            }
            *output_ << *value << "\n";
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

void CPU::record(EventType type, const std::string& detail) {
    if (!eventLog_) {
        return;
    }
    eventLog_->record(type, currentPid_, clock_ ? clock_->cycle() : 0, detail);
}

} // namespace aios