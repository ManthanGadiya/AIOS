#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace aios {

// ---------------------------------------------------------------------------
// Converts text program listings into memory words.
//   * "LOAD 10", "ADD 5", ... -> encoded instructions
//   * "42"                     -> raw data word
//   * "# comment" / empty      -> skipped
// (docs/15 section 3)
// ---------------------------------------------------------------------------
class ProgramLoader {
public:
    // Returns true on success and fills `out`; on failure returns false and
    // sets `error` to a human-readable message with the offending line.
    static bool parseProgram(const std::vector<std::string>& lines,
                             std::vector<int32_t>& out,
                             std::string& error);
};

} // namespace aios