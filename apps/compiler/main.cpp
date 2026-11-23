// ciph-compiler: compiles a ciph source file to bytecode.
//
//   ciph-compiler <source> -o <output>
//   ciph-compiler --keywords <file>
//
// Compiling writes results to files rather than the console, so a tool such
// as ciph-studio can start the compiler, wait for it and read them:
//
//   <output>       the bytecode, as raw bytes
//   <output>.lst   a disassembly listing, one instruction per line
//   <output>.err   only when compiling failed: the errors
//
// All three are removed before compiling, so what is on disk always belongs
// to the latest run.
//
// Exit codes: 0 success, 1 the source did not compile, 2 bad usage or a file
// could not be read or written.

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <ciph/compiler/compile_unit.hpp>
#include <ciph/compiler/error_defines.hpp>
#include <ciph/compiler/lexar_defines.hpp>
#include <ciph/shared/disassembler.hpp>

using namespace ciph;

namespace {

constexpr int exit_ok = 0;
constexpr int exit_compile_error = 1;
constexpr int exit_usage = 2;

void
print_usage() {
    fmt::print(stderr, "usage: ciph-compiler <source> -o <output>\n"
                       "       ciph-compiler --keywords <file>\n");
}

std::filesystem::path
with_suffix(const std::filesystem::path& path, std::string_view suffix) {
    std::filesystem::path result = path;
    result += suffix;
    return result;
}

// One error as "<source>:<line>:<column>: error: <what>", followed by any
// extra detail and nested errors, indented.
void
format_error(std::string& out, const std::filesystem::path& source, const Diagnostic& error, int depth) {
    const std::string indent(static_cast<std::size_t>(depth) * 2, ' ');

    std::string what;
    if (auto it = error_codes_map.find(error.code); it != error_codes_map.end()) {
        what = it->second;
    }
    else {
        what = fmt::format("error code 0x{:04x}", static_cast<uint16_t>(error.code));
    }

    out += fmt::format("{}{}:{}:{}: error: {}\n", indent, source.string(), error.position.line,
                       error.position.column, what);
    if (!error.additionalInfo.empty()) {
        out += fmt::format("{}  {}\n", indent, error.additionalInfo);
    }
    for (const Diagnostic& nested : error.nestedError) {
        format_error(out, source, nested, depth + 1);
    }
}

// Writes every error to <output>.err and to the console. Returns the exit
// code for a failed compile.
int
report_errors(const std::filesystem::path& source, const std::filesystem::path& error_path,
              std::span<const Diagnostic> errors) {
    std::string message;
    for (const Diagnostic& error : errors) {
        format_error(message, source, error, 0);
    }

    std::ofstream error_file(error_path, std::ios::binary);
    error_file << message;
    fmt::print(stderr, "{}", message);
    return exit_compile_error;
}

// Writes the language's keywords, one per line. ciph-studio reads this for
// syntax highlighting, so the list has a single owner: the lexer.
int
write_keywords(const std::filesystem::path& path) {
    std::ofstream out(path, std::ios::binary);
    for (const auto& [keyword, token] : s_keywords) {
        (void)token;
        out << keyword << '\n';
    }
    if (!out) {
        fmt::print(stderr, "ciph-compiler: could not write '{}'\n", path.string());
        return exit_usage;
    }
    return exit_ok;
}

int
compile(const std::filesystem::path& source_path, const std::filesystem::path& output_path) {
    const std::filesystem::path listing_path = with_suffix(output_path, ".lst");
    const std::filesystem::path error_path = with_suffix(output_path, ".err");

    std::error_code ignored;
    std::filesystem::remove(output_path, ignored);
    std::filesystem::remove(listing_path, ignored);
    std::filesystem::remove(error_path, ignored);

    std::ifstream source_file(source_path, std::ios::binary);
    if (!source_file) {
        fmt::print(stderr, "ciph-compiler: could not read '{}'\n", source_path.string());
        return exit_usage;
    }
    std::stringstream source;
    source << source_file.rdbuf();

    CompileUnit unit(source.str());

    // Each stage only runs if the one before it passed.
    if (!unit.parse() || !unit.semanticAnalyze() || !unit.generateCode()) {
        return report_errors(source_path, error_path, unit.readDiagnostics());
    }
    const std::vector<uint8_t>& bytecode = unit.readBytecode();

    std::ofstream output_file(output_path, std::ios::binary);
    output_file.write(reinterpret_cast<const char*>(bytecode.data()), static_cast<std::streamsize>(bytecode.size()));

    Disassembler disassembler(bytecode.data(), bytecode.size());
    disassembler.disassemble();
    std::ofstream listing_file(listing_path, std::ios::binary);
    for (const auto& [address, instruction] : disassembler.disassembledInstructions()) {
        // Each disassembled instruction already ends with a newline.
        fmt::print(listing_file, "0x{:04X}  {}", address, instruction);
    }

    const bool written = output_file.good() && listing_file.good();

    if (!written) {
        fmt::print(stderr, "ciph-compiler: could not write '{}'\n", output_path.string());
        return exit_usage;
    }
    return exit_ok;
}

} // namespace

int
main(int argc, char* argv[]) {
    if (argc == 3 && std::string_view(argv[1]) == "--keywords") {
        return write_keywords(argv[2]);
    }
    if (argc == 4 && std::string_view(argv[2]) == "-o") {
        return compile(argv[1], argv[3]);
    }
    print_usage();
    return exit_usage;
}
