// Windows implementation of ciph::platform::Process.
//
// A POSIX implementation would live in process_posix.cpp (fork + exec +
// waitpid) and be selected in this library's CMakeLists.txt.

#include <ciph/platform/process.hpp>

#include <ciph/platform/windows_command_line.hpp>

#include <string_view>
#include <utility>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

namespace ciph::platform {

namespace {

// Windows APIs take UTF-16; the rest of ciph uses UTF-8.
std::wstring
to_utf16(std::string_view utf8) {
    if (utf8.empty()) {
        return {};
    }
    const int length = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring utf16(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), utf16.data(), length);
    return utf16;
}

std::string
to_utf8(std::wstring_view utf16) {
    if (utf16.empty()) {
        return {};
    }
    const int length =
        WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), nullptr, 0, nullptr, nullptr);
    std::string utf8(static_cast<std::size_t>(length), '\0');
    WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), utf8.data(), length, nullptr,
                        nullptr);
    return utf8;
}

// The system's description of an error code, e.g. "The system cannot find the
// file specified".
std::string
describe_system_error(DWORD code) {
    wchar_t buffer[512];
    const DWORD length = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code, 0,
                                        buffer, static_cast<DWORD>(std::size(buffer)), nullptr);
    std::wstring_view text(buffer, length);

    // System messages end in ".\r\n"; drop that so it reads well mid-sentence.
    while (!text.empty() && (text.back() == L'\r' || text.back() == L'\n' || text.back() == L'.' ||
                             text.back() == L' ')) {
        text.remove_suffix(1);
    }
    return text.empty() ? std::string("unknown error") : to_utf8(text);
}

HANDLE
as_handle(std::uintptr_t handle) {
    return reinterpret_cast<HANDLE>(handle);
}

} // namespace

std::variant<ProcessError, Process>
Process::start(const ProcessOptions& options) {
    const std::string executable = to_utf8(options.executable.native());

    // CreateProcessW may modify the command line buffer, so it must be a
    // writable string and not a literal or a const view.
    std::wstring command_line = to_utf16(windows_command_line(executable, options.arguments));

    DWORD flags = 0;
    switch (options.console) {
        case ConsoleMode::inherit:
            break;
        case ConsoleMode::new_window:
            flags |= CREATE_NEW_CONSOLE;
            break;
        case ConsoleMode::none:
            flags |= CREATE_NO_WINDOW;
            break;
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION info{};

    const wchar_t* working_directory =
        options.working_directory.empty() ? nullptr : options.working_directory.c_str();

    // Passing the executable as the application name means it is run exactly
    // as given, with no search along PATH.
    const BOOL started = CreateProcessW(options.executable.c_str(), command_line.data(),
                                        nullptr, // default process security
                                        nullptr, // default thread security
                                        FALSE,   // the child does not inherit our handles
                                        flags,
                                        nullptr, // same environment as the parent
                                        working_directory, &startup, &info);
    if (!started) {
        const DWORD code = GetLastError();
        return ProcessError{"could not start '" + executable + "': " + describe_system_error(code) +
                            " (Windows error " + std::to_string(code) + ")"};
    }

    // We only ever ask about the process, never its main thread.
    CloseHandle(info.hThread);

    return Process(reinterpret_cast<std::uintptr_t>(info.hProcess));
}

Process::Process(Process&& other) noexcept
    : m_handle(std::exchange(other.m_handle, 0))
    , m_exit_code(other.m_exit_code) {}

Process&
Process::operator=(Process&& other) noexcept {
    if (this != &other) {
        close();
        m_handle = std::exchange(other.m_handle, 0);
        m_exit_code = other.m_exit_code;
    }
    return *this;
}

Process::~Process() {
    close();
}

void
Process::close() noexcept {
    if (m_handle != 0) {
        CloseHandle(as_handle(m_handle));
        m_handle = 0;
    }
}

int
Process::wait() {
    if (!m_exit_code.has_value() && m_handle != 0) {
        WaitForSingleObject(as_handle(m_handle), INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(as_handle(m_handle), &code);
        m_exit_code = static_cast<int>(code);
    }
    return m_exit_code.value_or(-1);
}

std::optional<int>
Process::exit_code() {
    if (!m_exit_code.has_value() && m_handle != 0) {
        // A zero timeout asks "has it finished?" without waiting. This is
        // more reliable than GetExitCodeProcess alone, which reports a
        // running process as exit code 259, a value a program may also
        // legitimately return.
        if (WaitForSingleObject(as_handle(m_handle), 0) == WAIT_OBJECT_0) {
            DWORD code = 0;
            GetExitCodeProcess(as_handle(m_handle), &code);
            m_exit_code = static_cast<int>(code);
        }
    }
    return m_exit_code;
}

} // namespace ciph::platform
