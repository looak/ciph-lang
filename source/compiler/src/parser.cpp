#include "parser.hpp"

#include <fmt/core.h>

#include "ast.hpp"

Parser::Parser(const std::string& input)
    : m_lexar(input) {
}

std::variant<ParserError, ASTBaseNode*>
Parser::parse() {
    auto [success, error_code] = m_lexar.lex();
    if (success == false) {
        ParserError error{.code = error_code, .position = {}, .additionalInfo = "Error while lexing input"};
        return error;
    }

    return parseProgram();
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseProgram() {
    ASTProgramNode* program = new ASTProgramNode();
    auto token = m_lexar.peek();

    while (token.readType() != TokenType::END_OF_FILE) {

        ASTBaseNode* statement = nullptr;
        auto statement_result = parseStatement();
        if (auto statement_ptr = std::get_if<ASTBaseNode*>(&statement_result)) {
            statement = *statement_ptr;
        }
        else {
            return std::get<ParserError>(statement_result);
        }

        program->addStatement(statement);
        token = m_lexar.peek();
    }
    return program;
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseStatement() {
    auto token = m_lexar.peek();
    switch (token.readType()) {
        case TokenType::RETURN:
            return parseReturnStatement();
        case TokenType::LET:
            return parseLetStatement();
        case TokenType::WHILE:
            return parseWhileStatement();
        default:
            return parseComparisonExpression();
    }
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseWhileStatement() {
    // m_lexar.pop();
    // if (m_lexar.expect(TokenType::OPEN_PAREN) == false) {}

    return nullptr;
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseReturnStatement() {
    m_lexar.pop();
    auto token = m_lexar.peek();
    ASTExpressionNode* expression = nullptr;
    if (token.readType() == TokenType::END_OF_FILE) {
        // return 0 as default;
        expression = new ASTNumericLiteralNode(0);
    }
    else {
        auto result = parseComparisonExpression();
        if (auto expr_ptr = std::get_if<ASTBaseNode*>(&result))
            expression = static_cast<ASTExpressionNode*>(*expr_ptr);
        else
            return std::get<ParserError>(result);
    }

    return new ASTReturnNode(expression);
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseComparisonExpression() {
    ASTExpressionNode* left = nullptr;
    ASTExpressionNode* right = nullptr;

    auto left_result = parseAddativeExpression();
    if (auto left_ptr = std::get_if<ASTBaseNode*>(&left_result)) {
        left = static_cast<ASTExpressionNode*>(*left_ptr);
    }
    else {
        return std::get<ParserError>(left_result);
    }

    auto op = m_lexar.peek();
    if (op.readOperator() == OperatorType::EQUAL || op.readOperator() == OperatorType::NOT_EQUAL ||
        op.readOperator() == OperatorType::LESS_THAN || op.readOperator() == OperatorType::GREATER_THAN) {
        m_lexar.pop();

        auto right_result = parseAddativeExpression();
        if (auto right_ptr = std::get_if<ASTBaseNode*>(&right_result)) {
            right = static_cast<ASTExpressionNode*>(*right_ptr);
        }
        else {
            return std::get<ParserError>(right_result);
        }

        return new ASTComparisonExpressionNode(left, right, op.readOperator());
    }

    return left;
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseAddativeExpression() {
    ASTExpressionNode* left = nullptr;
    ASTExpressionNode* right = nullptr;
    auto left_result = parseMultiplicativeExpression();
    if (auto left_ptr = std::get_if<ASTBaseNode*>(&left_result)) {
        left = static_cast<ASTExpressionNode*>(*left_ptr);
    }
    else {
        return std::get<ParserError>(left_result);
    }


    auto op = m_lexar.peek();
    if (op.readOperator() == OperatorType::ADDITION || op.readOperator() == OperatorType::SUBTRACTION) {
        m_lexar.pop();
        auto right_result = parseMultiplicativeExpression();
        if (auto right_ptr = std::get_if<ASTBaseNode*>(&right_result)) {
            right = static_cast<ASTExpressionNode*>(*right_ptr);
        }
        else {
            return std::get<ParserError>(right_result);
        }
        return new ASTBinaryExpressionNode(left, right, op.readOperator());
    }

    return left;
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseMultiplicativeExpression() {
    ASTExpressionNode* left = nullptr;
    ASTExpressionNode* right = nullptr;

    auto left_result = parsePrimaryExpression();
    if (auto left_ptr = std::get_if<ASTBaseNode*>(&left_result)) {
        left = static_cast<ASTExpressionNode*>(*left_ptr);
    }
    else {
        return std::get<ParserError>(left_result);
    }

    auto op = m_lexar.peek();
    if (op.readOperator() == OperatorType::MULTIPLICATION || op.readOperator() == OperatorType::DIVISION) {
        m_lexar.pop();

        auto right_result = parsePrimaryExpression();
        if (auto right_ptr = std::get_if<ASTBaseNode*>(&right_result)) {
            right = static_cast<ASTExpressionNode*>(*right_ptr);
        }
        else {
            return std::get<ParserError>(right_result);
        }

        return new ASTBinaryExpressionNode(left, right, op.readOperator());
    }

    return left;
}

std::variant<ParserError, ASTBaseNode*>
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

            ASTExpressionNode* expression = nullptr;
            auto expression_result = parseAddativeExpression();
            if (auto expression_ptr = std::get_if<ASTBaseNode*>(&expression_result)) {
                expression = static_cast<ASTExpressionNode*>(*expression_ptr);
            }
            else {
                return std::get<ParserError>(expression_result);
            }

            auto [success, expected_token] = m_lexar.popExpect(TokenType::CLOSE_PAREN);
            if (success == false) {
                ParserError error{.code = ErrorCode::SYNTAX_ERROR_EXPECTED_CLOSE_PARENTHESIS,
                                  .position = expected_token.readPosition(), // maybe should pass in token position here
                                  .additionalInfo = "Missmatch between open and close parenthesis in expression."};
                return error;
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

std::variant<ParserError, ASTBaseNode*>
Parser::parseLetStatement() {
    m_lexar.pop();
    auto [success, token] = m_lexar.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER,
                          .position = token.readPosition(),
                          .additionalInfo = "Expected identifier after let keyword"};
        return error;
    }

    std::string name = token.readValue();

    auto [op_success, op_token] = m_lexar.popOperator(OperatorType::ASSIGNMENT);
    if (op_success == false) {
        ParserError error{
            .code = ErrorCode::SYNTAX_ERROR_EXPECTED_OPERATOR,
            .position = op_token.readPosition(),
            .additionalInfo = fmt::format("Expected assignment operator after identifier {} in let statement", name)};
        return error;
    }

    ASTExpressionNode* expression = nullptr;
    auto expression_result = parseComparisonExpression();
    if (auto expression_ptr = std::get_if<ASTBaseNode*>(&expression_result)) {
        expression = static_cast<ASTExpressionNode*>(*expression_ptr);
    }
    else {
        return std::get<ParserError>(expression_result);
    }

    return new ASTLetNode(name, expression);
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseIdentifier() {
    auto [success, token] = m_lexar.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER,
                          .position = token.readPosition(),
                          .additionalInfo = "Expected identifier in expression"};
        return error;
    }

    std::string name = token.readValue();
    token = m_lexar.peek();

    if (token.readType() == TokenType::OPERATOR) {
        if (token.readOperator() == OperatorType::INCREMENT) {
            m_lexar.pop();
            return new ASTIdentifierNode(name, new ASTIncDecNode(true));
        }
        else if (token.readOperator() == OperatorType::DECREMENT) {
            m_lexar.pop();
            return new ASTIdentifierNode(name, new ASTIncDecNode(false));
        }
    }

    return new ASTIdentifierNode(name, nullptr);
}