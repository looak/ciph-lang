#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "error_defines.hpp"
#include "lexar_defines.hpp"

enum class CIPHTokenType
{
    // literals
    NUMBER,
    IDENTIFIER,

    // keywords
    LET,
    RETURN,
    WHILE,
    IF,
    ELSE,

    /*
STRING,
CHARACTER,
BOOLEAN,*/

    // scope and operators
    OPERATOR,
    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_BRACE,
    CLOSE_BRACE,
    OPEN_BRACKET,
    CLOSE_BRACKET,

    END_OF_FILE,
    UNKNOWN
};

class Token {
public:
    Token(const std::string& value, CIPHTokenType type);
    Token(const OperatorType& value, CIPHTokenType type);
    ~Token() = default;

    Token(const Token& other) = default;

    std::string readValue() const;
    OperatorType readOperator() const;
    CIPHTokenType readType() const;


    // TODO: should be moved to cpp file
    Token operator=(const Token& other) {
        m_value = other.m_value;
        m_type = other.m_type;
        return *this;
    }

    bool hasValue() const;

private:
    std::variant<std::string, OperatorType> m_value;
    CIPHTokenType m_type;
};

typedef std::pair<bool, ErrorCode> LexarResult;

class Lexar {
public:
    explicit Lexar(const std::string& input);
    ~Lexar() = default;

    /* @brief Performes the lexical analysis
     * @return Lexar result, a pair of a bool and error code		*/
    LexarResult lex();

    bool expect(CIPHTokenType type);
    bool popOperator(OperatorType op);
    Token peek();
    Token pop();
    // std::vector<std::string> getTokens() const;

private:
    char popNextChar();
    char peekNextChar();

    void skipWhiteSpaces();
    CIPHTokenType identifyKeyword(const std::string& value);
    std::optional<Token> identifyOperator(char cursor);
    bool isOperator(char cursor);

    std::string m_input;
    uint32_t m_position;
    Token m_peek;

    std::vector<Token> m_tokens;

    bool m_performedAnalysis;
};