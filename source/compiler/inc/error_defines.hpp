#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

enum class ErrorCode : uint16_t
{
    NO_ERROR = 0x0,
    SYNTAX_ERROR = 0xEF,
    SYNTAX_ERROR_EXPECTED_CLOSE_PARENTHESIS = 0xE001,
    SYNTAX_ERROR_EXPECTED_IDENTIFIER = 0xE002,
    SYNTAX_ERROR_EXPECTED_OPERATOR = 0xE003,

    LEXAR_PERFORMED_ANALYSIS = 0xA001,
    LEXAR_UNKNOWN_CHARACTER = 0xA002,
    ERROR_UNKNOWN = 0xFFFF
};


// can I some how define my errors to expect parameters, such as name of variable or line number?
const std::unordered_map<ErrorCode, std::string> error_codes_map = {
    {ErrorCode::SYNTAX_ERROR, "e0xef: syntax_error"},
    {ErrorCode::ERROR_UNKNOWN, "e0xffff: unknown_error"},
    {ErrorCode::LEXAR_PERFORMED_ANALYSIS, "e0xa001: lexar has already performed an analysis."},
    {ErrorCode::LEXAR_UNKNOWN_CHARACTER, "e0xa002: unsupported character in input: "},
    {ErrorCode::SYNTAX_ERROR_EXPECTED_CLOSE_PARENTHESIS, "e0xe001: expected close parenthesis. "},
    {ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER, "e0xe002: expected identifier. "},
    {ErrorCode::SYNTAX_ERROR_EXPECTED_OPERATOR, "e0xe003: expected operator. "}};


struct ErrorReport {
    ErrorCode code;
    std::string message;
    uint32_t line;
    uint32_t column;
};