#include <ciph/compiler/diagnostics.hpp>

#include <fmt/format.h>

using namespace ciph;

std::string
ciph::formatDiagnostic(std::string_view sourceName, const Diagnostic& diagnostic) {
    std::string out = fmt::format("{}:{}:{}: error: e0x{:x}: {}\n", sourceName, diagnostic.location.span.start.line,
                                  diagnostic.location.span.start.column, static_cast<uint16_t>(diagnostic.code),
                                  describe(diagnostic.code));
    if (!diagnostic.message.empty()) {
        out += fmt::format("  {}\n", diagnostic.message);
    }
    return out;
}

Diagnostic
ciph::diag::undeclaredIdentifier(Location location, std::string_view name) {
    return Diagnostic{.code = ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER,
                      .location = location,
                      .message = fmt::format("'{}' is not declared at this point", name)};
}

Diagnostic
ciph::diag::unknownCharacter(Location location, char character) {
    return Diagnostic{.code = ErrorCode::LEXER_UNKNOWN_CHARACTER,
                      .location = location,
                      .message = fmt::format("unsupported character '{}'", character)};
}
