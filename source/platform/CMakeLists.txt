# ciph::platform: the small amount of operating system specific code the tools
# need (starting processes; later, shared memory).
#
# Only Windows is implemented. To add a platform, write its source file next
# to process_win32.cpp and select it here.

ciph_add_library(platform
  SOURCES
    src/process_win32.cpp
    src/windows_command_line.cpp
  HEADERS
    include/ciph/platform/process.hpp
    include/ciph/platform/windows_command_line.hpp
)
