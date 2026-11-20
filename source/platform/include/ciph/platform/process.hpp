#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ciph::platform {

// Where a child process's console goes.
enum class ConsoleMode {
    inherit,    // share the parent's console, if it has one
    new_window, // open a console window of its own
    none        // no console at all; nothing is shown
};

struct ProcessOptions {
    // The executable file to run. It is not searched for on PATH.
    std::filesystem::path executable;

    // Passed to the child as argv[1..]. UTF-8.
    std::vector<std::string> arguments;

    // Directory the child starts in. Empty means the parent's.
    std::filesystem::path working_directory;

    ConsoleMode console = ConsoleMode::inherit;
};

// Why a process could not be started. `message` is complete and readable on
// its own: it names the executable and gives the operating system's reason.
struct ProcessError {
    std::string message;
};

// A running or finished child process.
//
// Owns the operating system's handle to the child and releases it on
// destruction. Destroying a Process does not stop or wait for the child; it
// only gives up the ability to ask about it.
//
// Move-only: there is exactly one owner of the handle.
class Process {
public:
    // Starts a child process. Returns as soon as it is started, without
    // waiting for it to finish.
    [[nodiscard]] static std::variant<ProcessError, Process> start(const ProcessOptions& options);

    Process(Process&& other) noexcept;
    Process& operator=(Process&& other) noexcept;
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    ~Process();

    // Blocks until the child has finished, then returns its exit code.
    int wait();

    // The child's exit code if it has finished, otherwise nothing. Never
    // blocks, so it is safe to call once per frame from a UI loop.
    [[nodiscard]] std::optional<int> exit_code();

private:
    explicit Process(std::uintptr_t handle) : m_handle(handle) {}
    void close() noexcept;

    // The native handle: a HANDLE on Windows. An integer wide enough for a
    // pointer keeps <windows.h> out of this header, and would hold a pid on
    // POSIX. Zero means "no process", which is the moved-from state.
    std::uintptr_t m_handle = 0;

    // Remembered once known, so the answer survives the handle being closed.
    std::optional<int> m_exit_code;
};

} // namespace ciph::platform
