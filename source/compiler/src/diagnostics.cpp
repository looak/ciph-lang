#include <ciph/compiler/diagnostics.hpp>

#include <fmt/format.h>

#include <ciph/compiler/lexer.hpp>

using namespace ciph;

namespace {

Diagnostic
make(ErrorCode code, Location location, std::string message) {
    return Diagnostic{.code = code, .location = location, .message = std::move(message)};
}

} // namespace

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

// ---- Lexer ----

Diagnostic
ciph::diag::unknownCharacter(Location location, char character) {
    return make(ErrorCode::LEXER_UNKNOWN_CHARACTER, location, fmt::format("unsupported character '{}'", character));
}

// ---- Parser ----

Diagnostic
ciph::diag::expectedExpression(Location location, const Token& found) {
    // Operator tokens carry no text, so only name what was found when there
    // is something to show.
    std::string message = "expected an expression";
    if (found.readType() == TokenType::END_OF_FILE) {
        message += " before the end of the file";
    }
    else if (!found.readStringValue().empty()) {
        message += fmt::format(", found '{}'", found.readStringValue());
    }
    return make(ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION, location, std::move(message));
}

Diagnostic
ciph::diag::unusedExpression(Location location) {
    return make(ErrorCode::SYNTAX_ERROR_UNUSED_EXPRESSION, location,
                "the result of this expression is not used; only a call, '++' or '--' can stand alone as a statement");
}

Diagnostic
ciph::diag::expectedSemicolon(Location location, std::string_view after) {
    return make(ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON, location, fmt::format("expected ';' after {}", after));
}

Diagnostic
ciph::diag::expectedParenthesis(Location location, char parenthesis, std::string_view where) {
    return make(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, location,
                fmt::format("expected '{}' {}", parenthesis, where));
}

Diagnostic
ciph::diag::expectedBrace(Location location, char brace, std::string_view where) {
    return make(ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH, location, fmt::format("expected '{}' {}", brace, where));
}

Diagnostic
ciph::diag::chainedComparison(Location location) {
    return make(ErrorCode::SYNTAX_ERROR_CHAINED_COMPARISON, location,
                "comparisons cannot be chained, as in 'a == b == c'; compare two values at a time");
}

Diagnostic
ciph::diag::numberOutOfRange(Location location, std::string_view digits) {
    return make(ErrorCode::SYNTAX_ERROR_NUMBER_OUT_OF_RANGE, location,
                fmt::format("{} does not fit in 16 bits; the largest number is 32767", digits));
}

Diagnostic
ciph::diag::expectedIdentifier(Location location, std::string_view where) {
    return make(ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER, location, fmt::format("expected a name {}", where));
}

Diagnostic
ciph::diag::expectedAssignment(Location location, std::string_view name) {
    return make(ErrorCode::SYNTAX_ERROR_EXPECTED_OPERATOR, location,
                fmt::format("expected '=' after '{}' in the let statement", name));
}

Diagnostic
ciph::diag::missingReturn(Location location, std::string_view function) {
    return make(ErrorCode::SYNTAX_ERROR_EXPECTED_RETURN_STATEMENT, location,
                fmt::format("function '{}' must end with a return statement", function));
}

// ---- Semantic analyzer ----

Diagnostic
ciph::diag::undeclaredIdentifier(Location location, std::string_view name) {
    return make(ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER, location,
                fmt::format("'{}' is not declared at this point", name));
}

Diagnostic
ciph::diag::undeclaredFunction(Location location, std::string_view name) {
    return make(ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER, location,
                fmt::format("there is no function named '{}'", name));
}

Diagnostic
ciph::diag::redeclaredVariable(Location location, std::string_view name, Position first) {
    return make(ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER, location,
                fmt::format("'{}' is already declared at line {}, column {}", name, first.line, first.column));
}

Diagnostic
ciph::diag::redeclaredFunction(Location location, std::string_view name, Position first) {
    return make(ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER, location,
                fmt::format("function '{}' is already defined at line {}, column {}", name, first.line,
                            first.column));
}

// ---- Code generator ----

Diagnostic
ciph::diag::functionsUnsupported(Location location, std::string_view function) {
    return make(ErrorCode::CODEGEN_ERROR_UNSUPPORTED, location,
                fmt::format("function '{}' cannot be compiled: functions are not supported yet", function));
}

Diagnostic
ciph::diag::nameInSeveralBlocksUnsupported(Location location, std::string_view name) {
    return make(ErrorCode::CODEGEN_ERROR_UNSUPPORTED, location,
                fmt::format("'{}' is declared in more than one block, which cannot be compiled yet", name));
}

Diagnostic
ciph::diag::tooManyVariables(Location location, std::string_view name) {
    return make(ErrorCode::CODEGEN_ERROR_TOO_MANY_VALUES, location,
                fmt::format("no room left for '{}': a variable's stack offset has to fit in one byte", name));
}

Diagnostic
ciph::diag::internalError(Location location, std::string detail) {
    return make(ErrorCode::CODEGEN_ERROR_INTERNAL, location, std::move(detail));
}
