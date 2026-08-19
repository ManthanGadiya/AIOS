#include <aios/memory/ProgramLoader.hpp>

#include <aios/core/types.hpp>

#include <string>

namespace aios {

bool ProgramLoader::parseProgram(const std::vector<std::string>& lines,
                                 std::vector<int32_t>& out,
                                 std::string& error) {
    out.clear();
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& raw = lines[i];

        // Trim surrounding whitespace.
        const size_t first = raw.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            continue; // blank line
        }
        const size_t last = raw.find_last_not_of(" \t\r\n");
        const std::string line = raw.substr(first, last - first + 1);

        if (line.empty() || line[0] == '#') {
            continue; // comment
        }

        Instruction instr;
        if (Instruction::parse(line, instr)) {
            out.push_back(instr.toWord());
            continue;
        }

        // Not an instruction: try a raw data word ("42", "-7", ...).
        try {
            size_t consumed = 0;
            const long value = std::stol(line, &consumed);
            if (consumed != line.size()) {
                throw std::invalid_argument("trailing characters");
            }
            out.push_back(static_cast<int32_t>(value));
            continue;
        } catch (const std::exception&) {
            error = "line " + std::to_string(i + 1) + ": \"" + line + "\"";
            return false;
        }
    }
    error.clear();
    return true;
}

} // namespace aios