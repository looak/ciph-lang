#pragma once
#include <cstdint>
#include <map>
#include <string>

enum class ErrorCode : uint16_t
{
    CIPH_NO_ERROR = 0x0,
    CIPH_SYNTAX_ERROR = 0xEF,

    CIPH_LEXAR_PERFORMED_ANALYSIS = 0xA001,
    CIPH_LEXAR_UNKNOWN_CHARACTER = 0xA002,
    CIPH_ERROR_UNKNOWN = 0xFFFF
};


// const std::map<ErrorCode, std::string> buildErrorCodeMap() {
//     return {};
// }

// can I some how define my errors to expect parameters, such as name of variable or line number?
// const static std::map<ErrorCode, std::string> error_codes_map = buildErrorCodeMap();
//{
// {ErrorCode::CIPH_SYNTAX_ERROR, "e0xef: syntax_error"},
// {ErrorCode::CIPH_ERROR_UNKNOWN, "e0xffff: unknown_error"},
// {ErrorCode::CIPH_LEXAR_PERFORMED_ANALYSIS, "e0xa001: lexar has already performed an analysis"},
// {ErrorCode::CIPH_LEXAR_UNKNOWN_CHARACTER, "e0xa002: unsupported character in input: "}
//};
