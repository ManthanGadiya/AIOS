#include <aios/core/types.hpp>
#include <aios/memory/ProgramLoader.hpp>

#include <doctest/doctest.h>

using namespace aios;

TEST_CASE("T-LDR-001: parses instructions, raw words and comments") {
    const std::vector<std::string> lines = {
        "LOAD 10",   // ACC = mem[10]
        "ADD 3",     // ACC = ACC + 3
        "42",        // raw data word
        "# a comment",
        "",
        "HALT",
    };
    std::vector<int32_t> out;
    std::string error;
    REQUIRE(ProgramLoader::parseProgram(lines, out, error));

    REQUIRE(out.size() == 4);
    CHECK(out[0] == Instruction{Opcode::LOAD, 10}.toWord());
    CHECK(out[1] == Instruction{Opcode::ADD, 3}.toWord());
    CHECK(out[2] == 42);
    CHECK(out[3] == Instruction{Opcode::HALT, 0}.toWord());
    CHECK(error.empty());
}

TEST_CASE("T-LDR-002: comments-only and blank input yields empty program") {
    const std::vector<std::string> lines = {"", "   ", "# nothing here"};
    std::vector<int32_t> out;
    std::string error;
    REQUIRE(ProgramLoader::parseProgram(lines, out, error));
    CHECK(out.empty());
    CHECK(error.empty());
}

TEST_CASE("T-LDR-003: malformed line fails with line number and text") {
    const std::vector<std::string> lines = {
        "LOAD 0",
        "NOT AN INSTRUCTION !!",
    };
    std::vector<int32_t> out;
    std::string error;
    CHECK_FALSE(ProgramLoader::parseProgram(lines, out, error));
    CHECK(error.find("line 2") != std::string::npos);
    CHECK(error.find("NOT AN INSTRUCTION !!") != std::string::npos);
}

TEST_CASE("T-LDR-004: negative raw data words parse") {
    const std::vector<std::string> lines = {"-7"};
    std::vector<int32_t> out;
    std::string error;
    REQUIRE(ProgramLoader::parseProgram(lines, out, error));
    REQUIRE(out.size() == 1);
    CHECK(out[0] == -7);
}

TEST_CASE("T-LDR-005: HALT without operand parses to one word") {
    const std::vector<std::string> lines = {"HALT"};
    std::vector<int32_t> out;
    std::string error;
    REQUIRE(ProgramLoader::parseProgram(lines, out, error));
    REQUIRE(out.size() == 1);
    CHECK(Instruction::fromWord(out[0]).opcode == Opcode::HALT);
}