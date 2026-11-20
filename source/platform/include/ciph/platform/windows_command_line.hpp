#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ciph::platform {

// Windows hands a new process one command line string, not a list of
// arguments; the child's C runtime splits it back into argv. These functions
// build that string so every argument survives the round trip unchanged,
// including ones with spaces, quotes and trailing backslashes.
//
// They are plain string functions with no dependency on <windows.h>, so they
// can be unit tested on any platform.

// Quotes one argument, if it needs it, following the rules the Microsoft C
// runtime uses to parse a command line.
std::string quote_windows_argument(std::string_view argument);

// The full command line: the executable followed by each argument, quoted as
// needed and separated by spaces.
std::string windows_command_line(std::string_view executable, const std::vector<std::string>& arguments);

} // namespace ciph::platform
