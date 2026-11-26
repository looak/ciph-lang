#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/lexer_defines.hpp>

namespace ciph {

class Token {
public:
    Token(std::string_view stringValue, TokenType type, Position position, uint32_t length, uint32_t index)
        : m_type(type), m_stringValue(stringValue), m_position(position), m_length(length), m_index(index) {}
    Token(OperatorType operatorValue, TokenType type, Position position, uint32_t length, uint32_t index)
        : m_type(type), m_operatorValue(operatorValue), m_position(position), m_length(length), m_index(index) {}
    // `length` is how many characters of source the token covers, starting at
    // `position`. Together they give the token's extent on its line.
    TokenType readType() const;
    std::string_view readStringValue() const;
    OperatorType readOperatorValue() const;
    Position readPosition() const;
    uint32_t readLength() const;
    // The characters the token covers, for a diagnostic. Tokens never span
    // lines, so the end is on the same line.
    Location readLocation() const;
    uint32_t readIndex() const;

private:
    TokenType m_type;
    
    std::string_view m_stringValue;
    OperatorType m_operatorValue = OperatorType::INVALID;
    
    Position m_position;
    uint32_t m_length;
    uint32_t m_index;
};

// A token is copied around freely (peek and pop return it by value), so it
// must stay a plain block of bytes. This fails if a member that owns
// something, such as a std::string, is added.
static_assert(std::is_trivially_copyable_v<Token>);

class Lexer {
public:
    explicit Lexer(std::string_view input);
    ~Lexer() = default;

    /**
    * @brief Performs the lexical analysis on the input string.
    *
    * This function processes the input string and converts it into a stream of tokens. It checks the input for
    * lexical errors and returns a `LexerResult` indicating the success or failure of the lexical analysis.
    * If the analysis is successful, the first element of the `LexerResult` is true and the second element is
    * an error code indicating no error. If the analysis fails, the first element of the `LexerResult` is false
    * and the second element is an error code indicating the type of error that occurred.
    *
    * @return A pair where the first element is a boolean indicating whether the analysis failed or not, if not the
    * second element will hold a error code. */
    std::pair<bool, ErrorCode> lex();

    // Every token, in source order, ending with END_OF_FILE. Valid after lex().
    // Walk them with a TokenCursor.
    const std::vector<Token>& readTokens() const { return m_tokens; }

private:
    // Adds a token starting at `position` and covering `length` characters,
    // then moves `position` one column on.
    void push(std::variant<std::string_view, OperatorType> value, TokenType type, Position& position, uint32_t length);
    char popNextChar();
    char peekNextChar();

    void skipWhiteSpaces(Position& cursorPos);

    bool identifyOperator(char cursor, Position& cursorPos);
    bool isOperator(char cursor);

    std::string_view m_input;
    uint32_t m_position;

    std::vector<Token> m_tokens;

    bool m_performedAnalysis;
    uint32_t m_index;
};

} // namespace ciph