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

    // Turns the whole input into tokens. A character that starts no token is
    // reported in readDiagnostics() and kept as an UNKNOWN token, and lexing
    // carries on, so the token list is always complete. Runs only once; later
    // calls do nothing.
    void lex();

    // lex() has run, whether or not it found errors.
    bool hasLexed() const { return m_hasLexed; }
    // lex() found something it could not turn into a token.
    bool hasErrors() const { return !m_diagnostics.empty(); }
    const std::vector<Diagnostic>& readDiagnostics() const { return m_diagnostics; }

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
    std::vector<Diagnostic> m_diagnostics;

    bool m_hasLexed;
    uint32_t m_index;
};

} // namespace ciph