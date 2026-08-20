#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/cpu/CPU.hpp>
#include <aios/interrupt/InterruptManager.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/memory/MemoryManager.hpp>
#include <aios/process/ProcessManager.hpp>

#include <doctest/doctest.h>

#include <deque>
#include <sstream>
#include <vector>

using namespace aios;

namespace {

// Paged wiring: memory + memory manager + event log + clock + CPU + interrupt
// manager + process manager, all owned by the test (docs/07 section 42).
struct Harness {
    Memory memory;
    MemoryManager mm;
    EventLog log;
    SimulationClock clock;
    CPU cpu;
    InterruptManager im;
    ProcessManager pm;
    std::ostringstream output;
    std::deque<int32_t> input;

    Harness() {
        mm.setMemory(&memory);
        mm.setEventLog(&log);
        mm.setClock(&clock);
        mm.initialize(4, 8); // docs/07 section 5 defaults

        pm.setMemory(&memory);
        pm.setEventLog(&log);
        pm.setCPU(&cpu);
        pm.setClock(&clock);
        pm.setMemoryManager(&mm);

        cpu.setMemory(&memory);
        cpu.setMemoryManager(&mm);
        cpu.setEventLog(&log);
        cpu.setClock(&clock);
        cpu.setInterruptManager(&im);
        cpu.setOutputStream(&output);
        cpu.setInputQueue(&input);

        im.setEventLog(&log);
        im.setProcessManager(&pm);
        im.setMemoryManager(&mm);
    }

    bool hasEvent(EventType type) const {
        for (const auto& e : log.events()) {
            if (e.type == type) {
                return true;
            }
        }
        return false;
    }
};

std::vector<int32_t> words(std::initializer_list<int32_t> list) {
    return std::vector<int32_t>(list);
}

} // namespace

TEST_CASE("T-PAGE-001: initialize sets page size, frame count and zero stats") {
    Harness h;
    const MemoryStatistics s = h.mm.getStatistics();
    CHECK(s.pageSize == 4);
    CHECK(s.totalFrames == 8);
    CHECK(s.totalWords == 32);
    CHECK(s.usedFrames == 0);
    CHECK(s.usedWords == 0);
    CHECK(s.pageFaultCount == 0);
    CHECK(s.swapWords == 0);
}

TEST_CASE("T-PAGE-002: allocateProcessMemory builds a non-resident page table") {
    Harness h;
    REQUIRE(h.mm.allocateProcessMemory(1, words({1, 2, 3, 4, 5, 6, 7, 8, 9})));

    const PageTable* pt = h.mm.getPageTable(1);
    REQUIRE(pt != nullptr);
    CHECK(pt->pid == 1);
    CHECK(pt->pageCount == 3); // 9 words / 4-word pages
    REQUIRE(pt->entries.size() == 3);
    for (const auto& e : pt->entries) {
        CHECK_FALSE(e.present); // demand paging: nothing resident yet
        CHECK(e.valid);
    }

    const MemoryStatistics s = h.mm.getStatistics();
    CHECK(s.swapWords == 12); // padded to full pages
    CHECK(s.usedFrames == 0);
    CHECK(h.hasEvent(EventType::PAGE_ALLOCATED));
}

TEST_CASE("T-PAGE-003: accessMemory translates a resident page and reads values") {
    Harness h;
    REQUIRE(h.mm.allocateProcessMemory(1, words({10, 20, 30, 40})));
    REQUIRE(h.mm.handlePageFault(1, 0));

    const MemoryAccessResult r0 = h.mm.accessMemory(1, 0, false, 0);
    CHECK(r0.status == MemoryAccessStatus::OK);
    CHECK(r0.value == 10);
    CHECK(r0.physicalAddress == 0); // frame 0 base

    const MemoryAccessResult r2 = h.mm.accessMemory(1, 2, false, 0);
    CHECK(r2.status == MemoryAccessStatus::OK);
    CHECK(r2.value == 30);
    CHECK(h.hasEvent(EventType::PAGE_LOADED));
    CHECK(h.hasEvent(EventType::PAGE_ACCESSED));
}

TEST_CASE("T-PAGE-004: accessMemory write stores the value and marks the page dirty") {
    Harness h;
    REQUIRE(h.mm.allocateProcessMemory(1, words({1, 2, 3, 4})));
    REQUIRE(h.mm.handlePageFault(1, 0));

    const MemoryAccessResult w = h.mm.accessMemory(1, 1, true, 99);
    CHECK(w.status == MemoryAccessStatus::OK);

    const MemoryAccessResult r = h.mm.accessMemory(1, 1, false, 0);
    CHECK(r.status == MemoryAccessStatus::OK);
    CHECK(r.value == 99);

    const PageTable* pt = h.mm.getPageTable(1);
    REQUIRE(pt != nullptr);
    CHECK(pt->entries[0].dirty);
    CHECK(h.mm.getStatistics().usedFrames == 1);
    CHECK(h.mm.getStatistics().usedWords == 4);
}

TEST_CASE("T-PAGE-005: handlePageFault loads the faulting page from swap") {
    Harness h;
    REQUIRE(h.mm.allocateProcessMemory(1, words({1, 2, 3, 4, 5, 6, 7, 8, 9})));
    CHECK(h.mm.getStatistics().swapWords == 12);

    REQUIRE(h.mm.handlePageFault(1, 1)); // second page (words 4-7)
    const MemoryAccessResult r = h.mm.accessMemory(1, 4, false, 0);
    CHECK(r.status == MemoryAccessStatus::OK);
    CHECK(r.value == 5);
    CHECK(r.physicalAddress == 0); // first free frame (frame 0) is used
}

TEST_CASE("T-PAGE-006: accessMemory rejects invalid addresses") {
    Harness h;
    REQUIRE(h.mm.allocateProcessMemory(1, words({1, 2, 3, 4})));

    CHECK(h.mm.accessMemory(1, 100, false, 0).status == MemoryAccessStatus::INVALID);
    CHECK(h.mm.accessMemory(999, 0, false, 0).status == MemoryAccessStatus::INVALID);
    CHECK_FALSE(h.mm.isValidLogicalAddress(1, 100));
    CHECK(h.mm.isValidLogicalAddress(1, 3));
}

TEST_CASE("T-PAGE-007: a page fault increments statistics and records an event") {
    Harness h;
    REQUIRE(h.mm.allocateProcessMemory(1, words({1, 2, 3, 4})));

    const MemoryAccessResult r = h.mm.accessMemory(1, 0, false, 0);
    CHECK(r.status == MemoryAccessStatus::PAGE_FAULT);
    CHECK(r.page == 0);

    const MemoryStatistics s = h.mm.getStatistics();
    CHECK(s.pageFaultCount == 1);
    CHECK(s.perProcessFaults.at(1) == 1);
    CHECK(h.hasEvent(EventType::PAGE_FAULT));
}

TEST_CASE("T-PAGE-008: FIFO replacement evicts the oldest resident page") {
    Harness h;
    h.mm.initialize(4, 2); // only two frames -> third fault must evict
    REQUIRE(h.mm.allocateProcessMemory(1, words({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12})));

    REQUIRE(h.mm.handlePageFault(1, 0));
    REQUIRE(h.mm.handlePageFault(1, 1));
    REQUIRE(h.mm.handlePageFault(1, 2)); // evicts page 0 (oldest)

    const PageTable* pt = h.mm.getPageTable(1);
    REQUIRE(pt != nullptr);
    CHECK_FALSE(pt->entries[0].present);
    CHECK(pt->entries[1].present);
    CHECK(pt->entries[2].present);

    const MemoryStatistics s = h.mm.getStatistics();
    CHECK(s.usedFrames == 2);
    CHECK(s.pageReplacementCount == 1);
    CHECK(h.hasEvent(EventType::PAGE_REPLACED));

    // Page 0 faults again and replaces page 1 (FIFO order 1, 2, 0).
    REQUIRE(h.mm.handlePageFault(1, 0));
    CHECK_FALSE(pt->entries[1].present);
    CHECK(pt->entries[0].present);
    CHECK(h.mm.getStatistics().pageReplacementCount == 2);
}

TEST_CASE("T-PAGE-009: a dirty victim page is written back to swap") {
    Harness h;
    h.mm.initialize(4, 1); // one frame forces replacement
    REQUIRE(h.mm.allocateProcessMemory(1, words({1, 2, 3, 4, 5, 6, 7, 8})));

    REQUIRE(h.mm.handlePageFault(1, 0));
    REQUIRE(h.mm.accessMemory(1, 1, true, 99).status == MemoryAccessStatus::OK); // dirty

    REQUIRE(h.mm.handlePageFault(1, 1)); // evicts dirty page 0, writes it back
    CHECK(h.mm.getStatistics().pageReplacementCount == 1);
    CHECK(h.hasEvent(EventType::PAGE_SWAPPED_OUT));

    // Evict the clean page 1, then reload page 0: the written-back 99 survives.
    REQUIRE(h.mm.handlePageFault(1, 0));
    const MemoryAccessResult r = h.mm.accessMemory(1, 1, false, 0);
    CHECK(r.status == MemoryAccessStatus::OK);
    CHECK(r.value == 99);
}

TEST_CASE("T-PAGE-010: releaseProcessMemory frees frames, page table and swap") {
    Harness h;
    h.mm.initialize(4, 1);
    REQUIRE(h.mm.allocateProcessMemory(1, words({1, 2, 3, 4})));
    REQUIRE(h.mm.handlePageFault(1, 0));
    CHECK(h.mm.getStatistics().usedFrames == 1);
    CHECK(h.mm.getStatistics().swapWords == 4);

    h.mm.releaseProcessMemory(1);
    CHECK(h.mm.getPageTable(1) == nullptr);
    CHECK(h.mm.getStatistics().usedFrames == 0);
    CHECK(h.mm.getStatistics().swapWords == 0);
    CHECK(h.mm.getStatistics().usedWords == 0);

    h.mm.releaseProcessMemory(999); // unknown pid is a no-op
}

TEST_CASE("T-PAGE-011: CPU runs a paged program end to end with faults") {
    Harness h;
    h.mm.initialize(4, 2);
    // Page 0: instructions. Page 1: data at logical 6.
    std::vector<int32_t> program = {
        Instruction{Opcode::LOAD, 6}.toWord(), Instruction{Opcode::WRITE, 6}.toWord(),
        Instruction{Opcode::HALT, 0}.toWord(), 0,
        0, 0, 42, 0};
    const int pid = h.pm.createProcess("paged", 1, program, 0);
    REQUIRE(pid != INVALID_PID);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));

    // Cycle 1: fetch page 0 faults, then OS loads it and the CPU retries.
    h.clock.tick();
    h.cpu.executeCycle();
    CHECK(h.cpu.state() == CpuState::INTERRUPTED);
    REQUIRE(h.im.serviceNextInterrupt());
    h.cpu.resume();

    // Cycle 2: LOAD 6 faults on page 1 (PC rolled back to retry the load).
    h.cpu.executeCycle();
    CHECK(h.cpu.state() == CpuState::INTERRUPTED);
    REQUIRE(h.im.serviceNextInterrupt());
    h.cpu.resume();

    // Cycle 3: retried LOAD 6 reads 42; then WRITE 6 outputs it; then HALT.
    h.cpu.executeCycle();
    CHECK(h.cpu.registers().acc == 42);
    h.cpu.executeCycle();
    h.cpu.executeCycle();
    CHECK(h.cpu.state() == CpuState::HALTED);
    CHECK(h.output.str() == "42\n");

    const MemoryStatistics s = h.mm.getStatistics();
    CHECK(s.pageFaultCount == 2);
    CHECK(s.perProcessFaults.at(pid) == 2);
    CHECK(s.usedFrames == 2);
}

TEST_CASE("T-PAGE-012: a fetch fault does not advance the PC") {
    Harness h;
    std::vector<int32_t> program = {Instruction{Opcode::LOAD, 2}.toWord(),
                                    Instruction{Opcode::HALT, 0}.toWord(), 42};
    const int pid = h.pm.createProcess("fetch-fault", 1, program, 0);
    REQUIRE(pid != INVALID_PID);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));

    h.cpu.executeCycle(); // fetch faults
    CHECK(h.cpu.state() == CpuState::INTERRUPTED);
    CHECK(h.cpu.registers().pc == 0); // unchanged for retry

    REQUIRE(h.im.serviceNextInterrupt());
    h.cpu.resume();
    h.cpu.executeCycle(); // retried fetch + LOAD 2
    CHECK(h.cpu.registers().pc == 1); // fetch advanced past LOAD; LOAD does not change PC
    CHECK(h.cpu.registers().acc == 42);
}

TEST_CASE("T-PAGE-013: invalid logical access fails the process via ERROR interrupt") {
    Harness h;
    std::vector<int32_t> program = {Instruction{Opcode::LOAD, 50}.toWord(),
                                    Instruction{Opcode::HALT, 0}.toWord(), 0, 0};
    const int pid = h.pm.createProcess("bad-access", 1, program, 0);
    REQUIRE(pid != INVALID_PID);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));

    h.cpu.executeCycle(); // fetch page 0 faults
    REQUIRE(h.im.serviceNextInterrupt());
    h.cpu.resume();
    h.cpu.executeCycle(); // LOAD 50 is outside the 1-page logical space
    CHECK(h.cpu.state() == CpuState::INTERRUPTED);
    CHECK(h.hasEvent(EventType::INVALID_MEMORY_ACCESS));

    REQUIRE(h.im.serviceNextInterrupt()); // ERROR -> markFailed
    CHECK(h.pm.getProcess(pid)->state == ProcessState::FAILED);
}

TEST_CASE("T-PAGE-014: legacy flat-memory path still works without MemoryManager") {
    Harness h;
    h.mm.initialize(4, 8);
    // Detach the memory manager: the process manager and CPU fall back to the
    // Stage I flat-Memory behavior (existing tests cover this; here we verify
    // the createProcess entry point keeps loading at the requested base).
    h.pm.setMemoryManager(nullptr);
    h.cpu.setMemoryManager(nullptr);
    h.im.setMemoryManager(nullptr);

    std::vector<int32_t> program = {Instruction{Opcode::LOAD, 0}.toWord(),
                                    Instruction{Opcode::HALT, 0}.toWord()};
    const int pid = h.pm.createProcess("flat", 1, program, 0);
    REQUIRE(pid != INVALID_PID);
    CHECK(h.memory.read(0) == Instruction{Opcode::LOAD, 0}.toWord());
    const auto* pcb = h.pm.getProcess(pid);
    REQUIRE(pcb != nullptr);
    CHECK(pcb->baseAddress == 0);
    CHECK(pcb->context.pc == 0);
}