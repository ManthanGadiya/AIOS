#include <aios/memory/Memory.hpp>

#include <doctest/doctest.h>

using namespace aios;

TEST_CASE("T-MEM-001: memory size is 1024 words and initially zero") {
    Memory mem;
    CHECK(mem.size() == Memory::MEMORY_SIZE);
    CHECK(mem.size() == 1024);
    REQUIRE(mem.read(0).has_value());
    CHECK(mem.read(0).value() == 0);
    CHECK(mem.read(1023).value() == 0);
}

TEST_CASE("T-MEM-002: write then read round trip") {
    Memory mem;
    CHECK(mem.write(42, 1234));
    REQUIRE(mem.read(42).has_value());
    CHECK(mem.read(42).value() == 1234);
}

TEST_CASE("T-MEM-003: loadProgram places words at base") {
    Memory mem;
    const std::vector<int32_t> program = {10, 11, 12};
    CHECK(mem.loadProgram(program, 100));
    CHECK(mem.read(100).value() == 10);
    CHECK(mem.read(101).value() == 11);
    CHECK(mem.read(102).value() == 12);
}

TEST_CASE("T-MEM-004: loadProgram rejects overflow") {
    Memory mem;
    const std::vector<int32_t> program(64, 7);
    CHECK_FALSE(mem.loadProgram(program, Memory::MEMORY_SIZE - 32));
    // Nothing was written.
    CHECK(mem.read(Memory::MEMORY_SIZE - 32).value() == 0);
}

TEST_CASE("T-MEM-005: reset zeroes all words") {
    Memory mem;
    mem.write(0, 1);
    mem.write(1023, 2);
    mem.reset();
    CHECK(mem.read(0).value() == 0);
    CHECK(mem.read(1023).value() == 0);
}