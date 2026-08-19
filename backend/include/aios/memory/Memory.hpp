#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace aios {

// ---------------------------------------------------------------------------
// Simulated main memory (docs/07).
// Flat array of 1024 int32 words. Unmapped / out-of-range accesses are
// reported via std::optional / bool instead of crashing.  [decision D1]
// ---------------------------------------------------------------------------
class Memory {
public:
    static constexpr uint32_t MEMORY_SIZE = 1024;

    Memory() : words_(MEMORY_SIZE, 0) {}

    std::optional<int32_t> read(uint32_t address) const {
        if (address >= MEMORY_SIZE) {
            return std::nullopt;
        }
        return words_[address];
    }

    bool write(uint32_t address, int32_t value) {
        if (address >= MEMORY_SIZE) {
            return false;
        }
        words_[address] = value;
        return true;
    }

    // Loads a program image at base; fails if it would overflow memory.
    bool loadProgram(const std::vector<int32_t>& program, uint32_t base) {
        if (base + program.size() > MEMORY_SIZE) {
            return false;
        }
        for (size_t i = 0; i < program.size(); ++i) {
            words_[base + i] = program[i];
        }
        return true;
    }

    void reset() { std::fill(words_.begin(), words_.end(), 0); }

    uint32_t size() const { return MEMORY_SIZE; }

private:
    std::vector<int32_t> words_;
};

} // namespace aios