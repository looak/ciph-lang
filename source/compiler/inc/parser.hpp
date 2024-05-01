#pragma once

#include <string>
#include <variant>

#include "error_defines.hpp"
#include "lexar.hpp"

class ASTBaseNode;
class ASTExpressionNode;
class ASTIdentifierNode;
class ASTLetNode;
class ASTProgramNode;
class ASTReturnNode;

struct ParserError
{
    ErrorCode code;
    Position position;
    // this string is supposed to be populated when the error is created, if needed.
    // default error messages are provided with the error code, and this will be appended to those.
    std::string additionalInfo;
};

class Parser {
public:
    explicit Parser(const std::string& input);
    ~Parser() = default;

    std::variant<ParserError, ASTBaseNode*> parse();
    std::variant<ParserError, ASTBaseNode*> parseProgram();

private:
    std::variant<ParserError, ASTBaseNode*> parseStatement();
    std::variant<ParserError, ASTBaseNode*> parseLetStatement();
    std::variant<ParserError, ASTBaseNode*> parseReturnStatement();
    std::variant<ParserError, ASTBaseNode*> parseWhileStatement();
    std::variant<ParserError, ASTBaseNode*> parseAddativeExpression();
    std::variant<ParserError, ASTBaseNode*> parseMultiplicativeExpression();
    std::variant<ParserError, ASTBaseNode*> parseIdentifier();
    std::variant<ParserError, ASTBaseNode*> parsePrimaryExpression();
    std::variant<ParserError, ASTBaseNode*> parseComparisonExpression();

    Lexar m_lexar;
};