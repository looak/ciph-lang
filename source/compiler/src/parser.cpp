#include <ciph/compiler/parser.hpp>

#include <fmt/core.h>

#include <ciph/compiler/ast.hpp>

using namespace ciph;

Parser::Parser(const std::string& input)
    : m_lexar(input) {
}

ParseExpect
Parser::parse() {
    auto [success, error_code] = m_lexar.lex();
    if (success == false) {
        ParserError error{.code = error_code, .position = {}, .additionalInfo = "Error while lexing input"};
        return std::unexpected(error);
    }

    return parseProgram();
}

ParseExpect
Parser::parseProgram() {
    ASTProgramNode* program = new ASTProgramNode();
    auto token = m_lexar.peek();

    while (token.readType() != TokenType::END_OF_FILE) {
        auto statement = parseStatement();
        if (!statement) {
            return statement;
        }

        program->addStatement(*statement);
        token = m_lexar.peek();
    }
    return program;
}

ParseExpect
Parser::parseStatement() {
    // Simple statements end with a semicolon; block statements (while, fn) do
    // not. and_then only runs the semicolon check when the statement itself
    // parsed, otherwise the statement's own error is passed through.
    auto semicolonAfter = [this](const char* what) {
        return [this, what](ASTBaseNode* statement) { return expectSemicolon(statement, what); };
    };

    auto token = m_lexar.peek();
    switch (token.readType()) {
        case TokenType::RETURN:
            return parseReturnStatement().and_then(semicolonAfter("return statement"));
        case TokenType::LET:
            return parseLetStatement().and_then(semicolonAfter("let statement"));
        case TokenType::WHILE:
            return parseWhileStatement();
        case TokenType::FUNCTION:
            return parseFunctionStatement();
        case TokenType::SEMICOLON: {
            ParserError error{.code = ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION,
                              .position = token.readPosition(),
                              .additionalInfo = "Found ';' with no statement before it"};
            return std::unexpected(error);
        }
        default:
            return parseComparisonExpression().and_then(semicolonAfter("expression"));
    }
}

ParseExpect
Parser::expectSemicolon(ASTBaseNode* statement, const char* what) {
    auto [success, token] = m_lexar.popExpect(TokenType::SEMICOLON);
    if (success == false) {
        // The position is that of the token found instead, which is often on
        // the next line. Pointing at the end of the statement needs token
        // lengths, which tokens do not carry yet.
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON,
                          .position = token.readPosition(),
                          .additionalInfo = fmt::format("Expected ';' after {}", what)};
        return std::unexpected(error);
    }
    return statement;
}

ParseExpect
Parser::parseWhileCondition() {
    auto [success, expected_token] = m_lexar.popExpect(TokenType::OPEN_PAREN);
    if (success == false) {
        ParserError error{
            .code = ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH,
            .position = expected_token.readPosition(),
            .additionalInfo = "Expecting a open parenthesis after while keyword which contains a expression"};
        return std::unexpected(error);
    }

    auto condition = parseComparisonExpression();
    if (!condition) {
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION,
                          .position = expected_token.readPosition(),
                          .additionalInfo = "Expecting while loop to be defined with a expression.",
                          .nestedError = {condition.error()}};
        return std::unexpected(error);
    }

    auto [success2, expected_token2] = m_lexar.popExpect(TokenType::CLOSE_PAREN);
    if (success2 == false) {
        ParserError error{
            .code = ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH,
            .position = expected_token2.readPosition(),
            .additionalInfo = "Expecting a close parenthesis after while keyword which contains a expression"};
        return std::unexpected(error);
    }

    return condition;
}

ParseExpect
Parser::parseScopeNode(ASTScopeNode* scopeNode) {
    auto [success, expected_token] = m_lexar.popExpect(TokenType::OPEN_BRACE);
    if (success == false) {
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH,
                          .position = expected_token.readPosition(),
                          .additionalInfo = "Expecting a open brace after keyword which contains a expression"};
        return std::unexpected(error);
    }

    Token token = m_lexar.peek();
    while (token.readType() != TokenType::CLOSE_BRACE) {
        auto statement = parseStatement();
        if (!statement) {
            return statement;
        }

        scopeNode->addStatement(*statement);

        token = m_lexar.peek();
        if (token.readType() == TokenType::END_OF_FILE) {
            ParserError error{.code = ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH,
                              .position = m_lexar.peek().readPosition(),
                              .additionalInfo = "Expecting a close brace before end of file."};
            return std::unexpected(error);
        }
    }

    auto [success1, expected_token1] = m_lexar.popExpect(TokenType::CLOSE_BRACE);
    if (success1 == false) {
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH,
                          .position = expected_token1.readPosition(),
                          .additionalInfo = "Expecting a close brace after open brance which contains statements"};
        return std::unexpected(error);
    }

    return scopeNode;
}

ParseExpect
Parser::parseWhileStatement() {
    m_lexar.pop();

    auto condition = parseWhileCondition();
    if (!condition) {
        return condition;
    }

    ASTWhileNode* whileNode = new ASTWhileNode(static_cast<ASTComparisonExpressionNode*>(*condition));

    auto body = parseScopeNode(whileNode);
    if (!body) {
        // create new error once we generalize while body into a scope parser
        return body;
    }

    return whileNode;
}

ParseExpect
Parser::parseReturnStatement() {
    m_lexar.pop();
    auto token = m_lexar.peek();
    ASTExpressionNode* expression = nullptr;
    if (token.readType() == TokenType::SEMICOLON) {
        // `return;` returns 0 as default
        expression = new ASTNumericLiteralNode(0);
    }
    else {
        auto result = parseComparisonExpression();
        if (!result) {
            return result;
        }
        expression = static_cast<ASTExpressionNode*>(*result);
    }

    return new ASTReturnNode(expression);
}

ParseExpect
Parser::parseComparisonExpression() {
    auto left_result = parseAddativeExpression();
    if (!left_result) {
        return left_result;
    }
    auto left = static_cast<ASTExpressionNode*>(*left_result);

    auto op = m_lexar.peek();
    if (op.readOperator() == OperatorType::EQUAL || op.readOperator() == OperatorType::NOT_EQUAL ||
        op.readOperator() == OperatorType::LESS_THAN || op.readOperator() == OperatorType::GREATER_THAN) {
        m_lexar.pop();

        auto right_result = parseAddativeExpression();
        if (!right_result) {
            return right_result;
        }
        auto right = static_cast<ASTExpressionNode*>(*right_result);

        return new ASTComparisonExpressionNode(left, right, op.readOperator());
    }

    return left;
}

ParseExpect
Parser::parseAddativeExpression() {
    auto left_result = parseMultiplicativeExpression();
    if (!left_result) {
        return left_result;
    }
    auto left = static_cast<ASTExpressionNode*>(*left_result);

    auto op = m_lexar.peek();
    if (op.readOperator() == OperatorType::ADDITION || op.readOperator() == OperatorType::SUBTRACTION) {
        m_lexar.pop();

        auto right_result = parseMultiplicativeExpression();
        if (!right_result) {
            return right_result;
        }
        auto right = static_cast<ASTExpressionNode*>(*right_result);

        return new ASTBinaryExpressionNode(left, right, op.readOperator());
    }

    return left;
}

ParseExpect
Parser::parseMultiplicativeExpression() {
    auto left_result = parsePrimaryExpression();
    if (!left_result) {
        return left_result;
    }
    auto left = static_cast<ASTExpressionNode*>(*left_result);

    auto op = m_lexar.peek();
    if (op.readOperator() == OperatorType::MULTIPLICATION || op.readOperator() == OperatorType::DIVISION) {
        m_lexar.pop();

        auto right_result = parsePrimaryExpression();
        if (!right_result) {
            return right_result;
        }
        auto right = static_cast<ASTExpressionNode*>(*right_result);

        return new ASTBinaryExpressionNode(left, right, op.readOperator());
    }

    return left;
}

ParseExpect
Parser::parsePrimaryExpression() {
    Token token = m_lexar.peek();

    switch (token.readType()) {
        case TokenType::NUMBER:
            m_lexar.pop();
            return new ASTNumericLiteralNode(static_cast<int16_t>(std::stoi(token.readValue())));
        case TokenType::IDENTIFIER:
            return parseIdentifier();
        case TokenType::OPEN_PAREN: {
            m_lexar.pop();

            auto expression = parseAddativeExpression();
            if (!expression) {
                return expression;
            }

            auto [success, expected_token] = m_lexar.popExpect(TokenType::CLOSE_PAREN);
            if (success == false) {
                ParserError error{.code = ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH,
                                  .position = expected_token.readPosition(),
                                  .additionalInfo = "Missmatch between open and close parenthesis in expression."};
                return std::unexpected(error);
            }

            return expression;
        }
        case TokenType::OPERATOR: {
            if (token.readOperator() == OperatorType::INCREMENT) {
                m_lexar.pop();
                return new ASTIncDecNode(true);
            }
            else if (token.readOperator() == OperatorType::DECREMENT) {
                m_lexar.pop();
                return new ASTIncDecNode(false);
            }
        }

        default:
            return nullptr;
    }
}

ParseExpect
Parser::parseLetStatement() {
    m_lexar.pop();
    auto [success, token] = m_lexar.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        ParserError error{  .code = ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER,
                            .position = token.readPosition(),
                            .additionalInfo = "Expected identifier after let keyword"};
        return std::unexpected(error);
    }

    std::string name = token.readValue();

    auto [op_success, op_token] = m_lexar.popOperator(OperatorType::ASSIGNMENT);
    if (op_success == false) {
        ParserError error{
            .code = ErrorCode::SYNTAX_ERROR_EXPECTED_OPERATOR,
            .position = op_token.readPosition(),
            .additionalInfo = fmt::format("Expected assignment operator after identifier {} in let statement", name)};
        return std::unexpected(error);
    }

    auto expression = parseComparisonExpression();
    if (!expression) {
        return expression;
    }

    return new ASTLetNode(name, static_cast<ASTExpressionNode*>(*expression));
}

ParseExpect
Parser::parseIdentifier() {
    auto [success, token] = m_lexar.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        ParserError error{  .code = ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER,
                            .position = token.readPosition(),
                            .additionalInfo = "Expected identifier in expression"};
        return std::unexpected(error);
    }

    std::string name = token.readValue();
    token = m_lexar.peek();

    if (token.readType() == TokenType::OPEN_PAREN) {
        m_lexar.pop(); // pop open paren
        // handle arguments later
        auto [close_success, result] = m_lexar.popExpect(TokenType::CLOSE_PAREN);
        if (close_success == false) {
            ParserError error{  .code = ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH,
                                .position = result.readPosition(),
                                .additionalInfo = "Expected closing parenthesis for function call"};
            return std::unexpected(error);
        }
        return new ASTCallNode(name);
    }
    else if (token.readOperator() == OperatorType::INCREMENT) {
        m_lexar.pop();
        return new ASTIdentifierNode(name, new ASTIncDecNode(true));
    }
    else if (token.readOperator() == OperatorType::DECREMENT) {
        m_lexar.pop();
        return new ASTIdentifierNode(name, new ASTIncDecNode(false));
    }

    return new ASTIdentifierNode(name, nullptr);
}

ParseExpect
Parser::parseFunctionStatement() {
    m_lexar.pop();
    auto [success, token] = m_lexar.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        ParserError error{  .code = ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER,
                            .position = token.readPosition(),
                            .additionalInfo = "Expected function identifier after fn keyword"};
        return std::unexpected(error);
    }
    auto name = token.readValue();
    auto functionNode = new ASTFunctionNode(name);

    m_lexar.popExpect(TokenType::OPEN_PAREN);
    // handle parameters later
    m_lexar.popExpect(TokenType::CLOSE_PAREN);

    auto body = parseScopeNode(functionNode);
    if (!body) {
        return body;
    }

    const auto& statements = functionNode->readStatements();
    bool hasReturn = !statements.empty() && statements.back()->readType() == ASTNodeType::RETURN;
    if (hasReturn == false) {
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_EXPECTED_RETURN_STATEMENT,
                          .position = token.readPosition(),
                          .additionalInfo = "Expected function to have a return statement"};
        return std::unexpected(error);
    }

    return functionNode;
}
