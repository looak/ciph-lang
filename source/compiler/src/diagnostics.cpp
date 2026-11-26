#include <ciph/compiler/diagnostics.hpp>

#include <cstddef>

#include <fmt/format.h>

using namespace ciph;

namespace {

void
appendDiagnostic(std::string& out, std::string_view sourceName, const Diagnostic& diagnostic, int depth) {
    const std::string indent(static_cast<std::size_t>(depth) * 2, ' ');

    std::string what;
    if (auto it = error_codes_map.find(diagnostic.code); it != error_codes_map.end()) {
        what = it->second;
    }
    else {
        what = fmt::format("error code 0x{:04x}", static_cast<uint16_t>(diagnostic.code));
    }

    out += fmt::format("{}{}:{}:{}: error: {}\n", indent, sourceName, diagnostic.location.span.start.line,
                       diagnostic.location.span.start.column, what);
    if (!diagnostic.message.empty()) {
        out += fmt::format("{}  {}\n", indent, diagnostic.message);
    }
    for (const Diagnostic& nested : diagnostic.nestedError) {
        appendDiagnostic(out, sourceName, nested, depth + 1);
    }
}

} // namespace

std::string
ciph::formatDiagnostic(std::string_view sourceName, const Diagnostic& diagnostic) {
    std::string out;
    appendDiagnostic(out, sourceName, diagnostic, 0);
    return out;
}
