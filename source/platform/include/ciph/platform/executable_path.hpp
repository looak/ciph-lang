#pragma once

#include <filesystem>

namespace ciph::platform {

// The full path of the program that is currently running. Unlike argv[0] it
// does not depend on how the program was started, so it is a reliable way to
// find files that sit next to the executable.
//
// Returns an empty path if the operating system cannot say.
std::filesystem::path executable_path();

} // namespace ciph::platform
