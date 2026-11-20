#include "build_session.hpp"

#include <fstream>
#include <sstream>
#include <system_error>
#include <utility>
#include <variant>

namespace ciph::studio {

namespace {

#ifdef _WIN32
constexpr const char* executable_suffix = ".exe";
#else
constexpr const char* executable_suffix = "";
#endif

std::string
read_text_file(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::filesystem::path
with_suffix(const std::filesystem::path& path, const char* suffix) {
    std::filesystem::path result = path;
    result += suffix;
    return result;
}

} // namespace

BuildSession::BuildSession(std::filesystem::path tools_directory, std::filesystem::path work_directory)
    : m_compiler(tools_directory / (std::string("ciph-compiler") + executable_suffix))
    , m_vm(tools_directory / (std::string("ciph-vm") + executable_suffix))
    , m_work_directory(std::move(work_directory))
    , m_source_path(m_work_directory / "script.ciph")
    , m_bytecode_path(m_work_directory / "script.bin") {
    std::error_code ignored;
    std::filesystem::create_directories(m_work_directory, ignored);
}

std::vector<std::string>
BuildSession::query_keywords() {
    const std::filesystem::path keywords_path = m_work_directory / "keywords.txt";

    auto result = platform::Process::start({.executable = m_compiler,
                                            .arguments = {"--keywords", keywords_path.string()},
                                            .console = platform::ConsoleMode::none});
    auto* process = std::get_if<platform::Process>(&result);
    if (process == nullptr || process->wait() != 0) {
        return {};
    }

    std::vector<std::string> keywords;
    std::ifstream file(keywords_path);
    for (std::string line; std::getline(file, line);) {
        if (!line.empty()) {
            keywords.push_back(line);
        }
    }
    return keywords;
}

void
BuildSession::compile(std::string_view source) {
    if (m_state == BuildState::compiling) {
        return;
    }

    m_compiled_source = source;
    {
        std::ofstream file(m_source_path, std::ios::binary);
        file << source;
        if (!file) {
            m_state = BuildState::failed;
            m_output = "Could not write the script to '" + m_source_path.string() + "'.";
            return;
        }
    }

    auto result = platform::Process::start({.executable = m_compiler,
                                            .arguments = {m_source_path.string(), "-o", m_bytecode_path.string()},
                                            .console = platform::ConsoleMode::none});
    if (auto* error = std::get_if<platform::ProcessError>(&result)) {
        m_state = BuildState::failed;
        m_output = "The compiler did not start: " + error->message;
        return;
    }

    m_compile_process = std::get<platform::Process>(std::move(result));
    m_state = BuildState::compiling;
    m_output = "Compiling...";
}

void
BuildSession::update() {
    if (m_state != BuildState::compiling || !m_compile_process.has_value()) {
        return;
    }
    if (const std::optional<int> exit_code = m_compile_process->exit_code()) {
        m_compile_process.reset();
        finish_compile(*exit_code);
    }
}

void
BuildSession::finish_compile(int exit_code) {
    // Exit codes are documented at the top of apps/compiler/main.cpp.
    if (exit_code == 0) {
        m_state = BuildState::succeeded;
        m_output = "Compiled.\n\n" + read_text_file(with_suffix(m_bytecode_path, ".lst"));
        return;
    }

    m_state = BuildState::failed;
    if (exit_code == 1) {
        m_output = read_text_file(with_suffix(m_bytecode_path, ".err"));
        if (m_output.empty()) {
            m_output = "The script did not compile, but the compiler left no error report.";
        }
    }
    else {
        m_output = "The compiler failed with exit code " + std::to_string(exit_code) +
                   ". This is a problem running the compiler, not an error in the script.";
    }
}

bool
BuildSession::run() {
    if (m_state != BuildState::succeeded) {
        m_output = "There is nothing to run: compile the script first.";
        return false;
    }

    // The Process is dropped straight away. That only releases our handle; the
    // stepper keeps running in its window until the user closes it.
    auto result = platform::Process::start({.executable = m_vm,
                                            .arguments = {m_bytecode_path.string()},
                                            .console = platform::ConsoleMode::new_window});
    if (auto* error = std::get_if<platform::ProcessError>(&result)) {
        m_output = "The VM did not start: " + error->message;
        return false;
    }
    return true;
}

} // namespace ciph::studio
