#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "error_defines.hpp"
#include "lexar_defines.hpp"

namespace ciph {

struct Position
{
    uint32_t line;
    uint32_t column;
};

class Token {
public:
    Token(const std::string& value, TokenType type, Position position);
    Token(const OperatorType& value, TokenType type, Position position);
    Token(const std::variant<std::string, OperatorType>& value, TokenType type, Position position);

    ~Token() = default;

    Token(const Token& other) = default;
    Token operator=(const Token& other);

    std::string readValue() const;
    OperatorType readOperator() const;
    TokenType readType() const;
    Position readPosition() const;
    bool hasValue() const;

private:
    std::variant<std::string, OperatorType> m_value;
    TokenType m_type;
    Position m_position;
};

class Lexar {
public:
    explicit Lexar(const std::string& input);
    ~Lexar() = default;

    /**
    * @brief Performs the lexical analysis on the input string.
    *
    * This function processes the input string and converts it into a stream of tokens. It checks the input for
    * lexical errors and returns a `LexarResult` indicating the success or failure of the lexical analysis.
    * If the analysis is successful, the first element of the `LexarResult` is true and the second element is
    * an error code indicating no error. If the analysis fails, the first element of the `LexarResult` is false
    * and the second element is an error code indicating the type of error that occurred.
    *
    * @return A pair where the first element is a boolean indicating whether the analysis failed or not, if not the
    * second element will hold a error code. */
    std::pair<bool, ErrorCode> lex();

    /**
    * @brief Pops the next token from the token stream.
    *
    * This function removes the next token from the token stream and returns it. If there are no more tokens in the
    * stream, the behavior of this function is undefined. Callers should ensure that there is a next token by
    * calling `hasNext` before calling this function.
    *
    * @return The next token in the token stream. */
    Token pop();

    /**
    * @brief Pops the next token from the token stream and checks if its type matches the expected type.
    *
    * This function removes the next token from the token stream and checks if its type matches the type
    * specified by the 'type' parameter. If the types match, the function returns a pair where the first
    * element is true and the second element is the popped token. If the types do not match, the function
    * returns a pair where the first element is false and the second element is the popped token.
    *
    * @param type The expected type of the token.
    * @return A pair where the first element is a boolean indicating whether the token type matched the
    * expected type, and the second element is the popped token. */
    std::pair<bool, Token> popExpect(TokenType type);

    /**
    * @brief Pops the next token from the token stream and checks if it's an operator of the expected type.
    *
    * This function removes the next token from the token stream and checks if it's an operator token of the type
    * specified by the 'op' parameter. If the token is an operator of the expected type, the function returns a pair
    * where the first element is true and the second element is the popped token. If the token is not an operator or
    * is an operator of a different type, the function returns a pair where the first element is false and the second
    * element is the popped token.
    *
    * @param op The expected operator type of the token.
    * @return A pair where the first element is a boolean indicating whether the token is an operator of the expected
    * type, and the second element is the popped token.  */
    std::pair<bool, Token> popOperator(OperatorType op);

    /**
    * @brief Peeks at the token in the token stream at the specified offset without removing it.
    *
    * This function returns the token at the specified offset in the token stream without removing it.
    * If no offset is specified, it returns the next token in the stream.
    *
    * @param offset The offset in the token stream of the token to peek at. Defaults to 0, which means the next token.
    * @return The token at the specified offset in the token stream. */
    Token peek(uint16_t offset = 0) const;

private:
    void push(std::variant<std::string, OperatorType> value, TokenType type, Position& position);
    char popNextChar();
    char peekNextChar();

    void skipWhiteSpaces(Position& cursorPos);

    bool identifyOperator(char cursor, Position& cursorPos);
    bool isOperator(char cursor);

    std::string m_input;
    uint32_t m_position;

    std::vector<Token> m_tokens;

    bool m_performedAnalysis;
};

} // namespace ciph