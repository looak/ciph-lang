#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ciph {

enum class ErrorCode : uint16_t {
    NO_ERR = 0x0,
    SYNTAX_ERROR = 0xEF,
    SYNTAX_ERROR_PARENTHESIS_MISSMATCH = 0xE001,
    SYNTAX_ERROR_BRACE_MISSMATCH = 0xE002,
    SYNTAX_ERROR_EXPECTED_IDENTIFIER = 0xE003,
    SYNTAX_ERROR_EXPECTED_OPERATOR = 0xE004,
    SYNTAX_ERROR_EXPECTED_EXPRESSION = 0xE005,
    SYNTAX_ERROR_EXPECTED_SEMICOLON = 0xE006,
    
    SYNTAX_ERROR_UNUSED_EXPRESSION = 0xE060,
    SYNTAX_ERROR_CHAINED_COMPARISON = 0xE061,
    
    SYNTAX_ERROR_EXPECTED_RETURN_STATEMENT = 0xE101, // functions need return statements

    SEMANTIC_ERROR_UNDECLARED_IDENTIFIER = 0xE201,
    SEMANTIC_ERROR_REDECLARED_IDENTIFIER = 0xE202,

    CODEGEN_ERROR_UNSUPPORTED = 0xE301,     // valid ciph the code generator cannot compile yet
    CODEGEN_ERROR_TOO_MANY_VALUES = 0xE302, // a limit of the bytecode format was reached
    CODEGEN_ERROR_INTERNAL = 0xE3FF,        // a bug in the compiler, not in the program

    LEXER_PERFORMED_ANALYSIS = 0xA001,
    LEXER_UNKNOWN_CHARACTER = 0xA002,
    ERROR_UNKNOWN = 0xFFFF
};


// can I some how define my errors to expect parameters, such as name of variable or line number?
const std::unordered_map<ErrorCode, std::string> error_codes_map = {
    {ErrorCode::SYNTAX_ERROR, "e0xef: syntax_error"},
    {ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, "e0xe001: parenthesis missmatch"},
    {ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH, "e0xe002: brace missmatch"},
    {ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER, "e0xe003: expected identifier"},
    {ErrorCode::SYNTAX_ERROR_EXPECTED_OPERATOR, "e0xe004: expected operator"},
    {ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION, "e0xe005: expected expression"},
    {ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON, "e0xe006: expected semicolon"},
    {ErrorCode::SYNTAX_ERROR_UNUSED_EXPRESSION, "e0xe060: expression result is not used"},
    {ErrorCode::SYNTAX_ERROR_CHAINED_COMPARISON, "e0xe061: comparisons cannot be chained"},
    {ErrorCode::SYNTAX_ERROR_EXPECTED_RETURN_STATEMENT, "e0xe101: expected return statement"},
    {ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER, "e0xe201: undeclared identifier"},
    {ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER, "e0xe202: redeclared identifier"},
    {ErrorCode::CODEGEN_ERROR_UNSUPPORTED, "e0xe301: not supported yet"},
    {ErrorCode::CODEGEN_ERROR_TOO_MANY_VALUES, "e0xe302: too many values on the stack"},
    {ErrorCode::CODEGEN_ERROR_INTERNAL, "e0xe3ff: internal code generator error"},
    {ErrorCode::LEXER_PERFORMED_ANALYSIS, "e0xa001: lexer has already performed an analysis"},
    {ErrorCode::LEXER_UNKNOWN_CHARACTER, "e0xa002: unsupported character in input"},
    {ErrorCode::ERROR_UNKNOWN, "e0xffff: unknown_error"}};


// A place in the source text: a line and a column on it.
struct Position
{
    uint32_t line;
    uint32_t column;
};

// One problem found in the program being compiled. Every stage reports its
// errors as these; `code` tells which stage it came from and what went wrong.
//
// A diagnostic owns everything it holds, so it stays valid after the compile
// unit it came from is gone.
struct Diagnostic {
    ErrorCode code;
    Position position;
    // Populated when the diagnostic is created, if needed. Default messages
    // come with the error code, and this is appended to those.
    std::string additionalInfo;
    // The errors that led to this one, such as the bad expression inside a
    // while condition.
    std::vector<Diagnostic> nestedError = {};
};

} // namespace ciph