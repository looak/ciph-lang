#pragma once
#include <cstdint>
#include <map>
#include <string>

enum class ErrorCode : uint16_t
{
    NO_ERROR = 0x0,
    SYNTAX_ERROR = 0xEF,

    LEXAR_PERFORMED_ANALYSIS = 0xA001,
    LEXAR_UNKNOWN_CHARACTER = 0xA002,
    ERROR_UNKNOWN = 0xFFFF
};


// can I some how define my errors to expect parameters, such as name of variable or line number?
std::map<ErrorCode, std::string> error_codes_map = {
    {ErrorCode::SYNTAX_ERROR, "e0xef: syntax_error"},
    {ErrorCode::ERROR_UNKNOWN, "e0xffff: unknown_error"},
    {ErrorCode::LEXAR_PERFORMED_ANALYSIS, "e0xa001: lexar has already performed an analysis"},
    {ErrorCode::LEXAR_UNKNOWN_CHARACTER, "e0xa002: unsupported character in input: "}};
