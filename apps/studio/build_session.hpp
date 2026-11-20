#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <ciph/platform/process.hpp>

namespace ciph::studio {

enum class BuildState {
    idle,      // nothing compiled yet
    compiling, // the compiler is running
    succeeded,
    failed
};

// Drives the ciph tools as separate programs: compiling a script and running
// the result. The studio never links the compiler or the VM, so either can be
// rebuilt while the studio stays open.
//
// Has no dependency on the UI.
class BuildSession {
public:
    // `tools_directory` holds ciph-compiler and ciph-vm; `work_directory` is
    // where the script and the compiler's output files are written.
    BuildSession(std::filesystem::path tools_directory, std::filesystem::path work_directory);

    // Asks the compiler for the language's keywords. Blocks briefly. Returns
    // an empty list if the compiler could not be run.
    std::vector<std::string> query_keywords();

    // Starts compiling `source` and returns immediately. Does nothing if a
    // compile is already running. Call update() to find out when it finishes.
    void compile(std::string_view source);

    // Checks whether a running compile has finished and, if so, collects its
    // results. Never blocks; call once per frame.
    void update();

    // Opens the VM's stepper on the last successful compile, in a console
    // window of its own. Returns false, with the reason in output(), if there
    // is nothing to run or the VM could not be started.
    bool run();

    [[nodiscard]] BuildState state() const { return m_state; }

    // Text to show the user: the listing after a successful compile, or what
    // went wrong.
    [[nodiscard]] const std::string& output() const { return m_output; }

    // The source of the most recent compile, to tell whether the script has
    // been edited since.
    [[nodiscard]] const std::string& compiled_source() const { return m_compiled_source; }

private:
    void finish_compile(int exit_code);

    std::filesystem::path m_compiler;
    std::filesystem::path m_vm;
    std::filesystem::path m_work_directory;
    std::filesystem::path m_source_path;
    std::filesystem::path m_bytecode_path;

    BuildState m_state = BuildState::idle;
    std::string m_output;
    std::string m_compiled_source;
    std::optional<platform::Process> m_compile_process;
};

} // namespace ciph::studio
