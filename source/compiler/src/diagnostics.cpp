#include <ciph/compiler/diagnostics.hpp>

#include <fmt/format.h>

using namespace ciph;

std::string
ciph::formatDiagnostic(std::string_view sourceName, const Diagnostic& diagnostic) {
    std::string what;
    if (auto it = error_codes_map.find(diagnostic.code); it != error_codes_map.end()) {
        what = it->second;
    }
    else {
        what = fmt::format("error code 0x{:04x}", static_cast<uint16_t>(diagnostic.code));
    }

    std::string out = fmt::format("{}:{}:{}: error: {}\n", sourceName, diagnostic.location.span.start.line,
                                  diagnostic.location.span.start.column, what);
    if (!diagnostic.message.empty()) {
        out += fmt::format("  {}\n", diagnostic.message);
    }
    return out;
}
