#include <aios/memory/Memory.hpp>

#include <doctest/doctest.h>

// Toolchain sanity check: the test harness compiles, links against the aios
// headers, and doctest executes.
TEST_CASE("smoke: memory read/write round trip") {
    aios::Memory mem;
    CHECK(mem.write(0, 42));
    REQUIRE(mem.read(0).has_value());
    CHECK(mem.read(0).value() == 42);
}

TEST_CASE("smoke: out-of-range memory access reports nullopt/false") {
    aios::Memory mem;
    CHECK_FALSE(mem.write(aios::Memory::MEMORY_SIZE, 1));
    CHECK_FALSE(mem.read(aios::Memory::MEMORY_SIZE).has_value());
}