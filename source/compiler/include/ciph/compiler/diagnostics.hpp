#pragma once
#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace ciph {

// Every error code, written once. Each entry is X(name, value, description).
// The enum, describe() and all_error_codes below are all made from this list.
//
// Comments in the list must be /* */: a // comment would swallow the
// line-continuation backslash.
#define CIPH_DIAGNOSTICS(X) \
    X(SYNTAX_ERROR,                          0xEF,   "syntax_error") \
    X(SYNTAX_ERROR_PARENTHESIS_MISSMATCH,     0xE001, "parenthesis missmatch") \
    X(SYNTAX_ERROR_BRACE_MISSMATCH,           0xE002, "brace missmatch") \
    X(SYNTAX_ERROR_EXPECTED_IDENTIFIER,       0xE003, "expected identifier") \
    X(SYNTAX_ERROR_EXPECTED_OPERATOR,         0xE004, "expected operator") \
    X(SYNTAX_ERROR_EXPECTED_EXPRESSION,       0xE005, "expected expression") \
    X(SYNTAX_ERROR_EXPECTED_SEMICOLON,        0xE006, "expected semicolon") \
    X(SYNTAX_ERROR_NUMBER_OUT_OF_RANGE,       0xE007, "number does not fit in 16 bits") \
    \
    X(SYNTAX_ERROR_UNUSED_EXPRESSION,         0xE060, "expression result is not used") \
    X(SYNTAX_ERROR_CHAINED_COMPARISON,        0xE061, "comparisons cannot be chained") \
    \
    X(SYNTAX_ERROR_EXPECTED_RETURN_STATEMENT, 0xE101, "expected return statement") /* functions need return statements */ \
    \
    X(SEMANTIC_ERROR_UNDECLARED_IDENTIFIER,   0xE201, "undeclared identifier") \
    X(SEMANTIC_ERROR_REDECLARED_IDENTIFIER,   0xE202, "redeclared identifier") \
    \
    X(CODEGEN_ERROR_UNSUPPORTED,              0xE301, "not supported yet")                /* valid ciph the code generator cannot compile yet */ \
    X(CODEGEN_ERROR_TOO_MANY_VALUES,          0xE302, "too many values on the stack")     /* a limit of the bytecode format was reached */ \
    X(CODEGEN_ERROR_INTERNAL,                 0xE3FF, "internal code generator error")    /* a bug in the compiler, not in the program */ \
    \
    X(LEXER_UNKNOWN_CHARACTER,                0xA002, "unsupported character in input") \
    X(ERROR_UNKNOWN,                          0xFFFF, "unknown_error")

enum class ErrorCode : uint16_t {
#define CIPH_X(NAME, VALUE, DESCRIPTION) NAME = VALUE,
    CIPH_DIAGNOSTICS(CIPH_X)
#undef CIPH_X
};

// Every error code, to loop over.
inline constexpr std::array all_error_codes{
#define CIPH_X(NAME, VALUE, DESCRIPTION) ErrorCode::NAME,
    CIPH_DIAGNOSTICS(CIPH_X)
#undef CIPH_X
};

// The fixed text that goes with an error code.
[[nodiscard]] constexpr std::string_view
describe(ErrorCode code) {
    switch (code) {
#define CIPH_X(NAME, VALUE, DESCRIPTION) case ErrorCode::NAME: return DESCRIPTION;
        CIPH_DIAGNOSTICS(CIPH_X)
#undef CIPH_X
    }
    return "unknown error code";
}


// A place in the source text: a line and a column on it.
struct Position
{
    uint32_t line;
    uint32_t column;
};

// A stretch of source text. Half-open: `end` is one column past the last
// character, so `start == end` is an empty span, a point between characters.
struct TextSpan
{
    Position start;
    Position end;
};

// Where a diagnostic points. Only the span for now; an id for the source it
// belongs to may join later.
struct Location
{
    TextSpan span;
};

// One problem found in the program being compiled. Every stage reports its
// errors as these; `code` tells which stage it came from and what went wrong.
//
// A diagnostic owns everything it holds, so it stays valid after the compile
// unit it came from is gone.
struct Diagnostic {
    ErrorCode code;
    Location location;
    // Populated when the diagnostic is created, if needed. Default messages
    // come with the error code, and this is appended to those.
    std::string message;
};

// The result of anything that can fail with a diagnostic. [[nodiscard]] on the
// type, so every function returning it is nodiscard without saying so.
//
// and_then, transform and or_else return a plain std::expected, which is not
// nodiscard and does not convert back to an Expect.
template <class T>
class [[nodiscard]] Expect : public std::expected<T, Diagnostic> {
public:
    using std::expected<T, Diagnostic>::expected;
};

// A diagnostic as plain text, for a console or a log:
//
//   <sourceName>:<line>:<column>: error: <what>
//     <additional info>
//
// Every line ends with a newline.
//
// This is the simplest way to show a diagnostic. `sourceName` is only printed,
// never opened, so it can be a file path or any other label.
[[nodiscard]] std::string formatDiagnostic(std::string_view sourceName, const Diagnostic& diagnostic);

class Token;

// One function per error, each making its diagnostic from what went wrong.
// Messages start in lower case, have no full stop, and quote code: 'x', '('.
namespace diag {

// Lexer
[[nodiscard]] Diagnostic unknownCharacter(Location location, char character);

// Parser. `where` finishes the sentence, such as "after 'while'".
[[nodiscard]] Diagnostic expectedExpression(Location location, const Token& found);
[[nodiscard]] Diagnostic unusedExpression(Location location);
[[nodiscard]] Diagnostic expectedSemicolon(Location location, std::string_view after);
[[nodiscard]] Diagnostic expectedParenthesis(Location location, char parenthesis, std::string_view where);
[[nodiscard]] Diagnostic expectedBrace(Location location, char brace, std::string_view where);
[[nodiscard]] Diagnostic chainedComparison(Location location);
[[nodiscard]] Diagnostic numberOutOfRange(Location location, std::string_view digits);
[[nodiscard]] Diagnostic expectedIdentifier(Location location, std::string_view where);
[[nodiscard]] Diagnostic expectedAssignment(Location location, std::string_view name);
[[nodiscard]] Diagnostic missingReturn(Location location, std::string_view function);

// Semantic analyzer
[[nodiscard]] Diagnostic undeclaredIdentifier(Location location, std::string_view name);

} // namespace diag

} // namespace ciph