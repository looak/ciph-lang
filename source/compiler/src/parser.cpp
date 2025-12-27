#include "parser.hpp"

#include <fmt/core.h>

#include "ast.hpp"

Parser::Parser(const std::string& input)
    : m_lexar(input) {
}

ASTBaseNode* Parser::parse() {
    return parseProgram();
}

ASTProgramNode* Parser::parseProgram() {
    ASTProgramNode* program = new ASTProgramNode();
    auto token = m_lexar.peek();

    while (token.readType() != CIPHTokenType::END_OF_FILE) {
        auto stmnt = parseStatement();
        if (stmnt != nullptr)
            program->addStatement(stmnt);
        token = m_lexar.peek();
    }
    return program;
}

ASTBaseNode* Parser::parseStatement() {
    auto token = m_lexar.peek();
    switch (token.readType()) {
        case CIPHTokenType::RETURN:
            return parseReturnStatement();
        case CIPHTokenType::LET:
            return parseLetStatement();
        case CIPHTokenType::WHILE:
            return parseWhileStatement();
        default:
            return parseComparisonExpression();
    }
}
ASTWhileNode* Parser::parseWhileStatement() {
    // m_lexar.pop();
    // m_lexar.expect()
    return nullptr;
}

ASTReturnNode* Parser::parseReturnStatement() {
    m_lexar.pop();
    auto token = m_lexar.peek();
    ASTExpressionNode* expression = nullptr;
    if (token.readType() == CIPHTokenType::END_OF_FILE) {
        // return 0 as default;
        expression = new ASTNumericLiteralNode(0);
    }
    else {
        expression = parseComparisonExpression();
    }

    return new ASTReturnNode(expression);
}

ASTExpressionNode* Parser::parseComparisonExpression() {
    ASTExpressionNode* left = parseAddativeExpression();
    ASTExpressionNode* right = nullptr;

    auto op = m_lexar.peek();
    if (op.readOperator() == OperatorType::EQUAL || op.readOperator() == OperatorType::NOT_EQUAL ||
        op.readOperator() == OperatorType::LESS_THAN || op.readOperator() == OperatorType::GREATER_THAN) {
        m_lexar.pop();
        right = parseAddativeExpression();
        return new ASTComparisonExpressionNode(left, right, op.readOperator());
    }

    return left;
}

ASTExpressionNode* Parser::parseAddativeExpression() {
    ASTExpressionNode* left = parseMultiplicativeExpression();
    ASTExpressionNode* right = nullptr;

    auto op = m_lexar.peek();

    if (op.readOperator() == OperatorType::ADDITION || op.readOperator() == OperatorType::SUBTRACTION) {
        m_lexar.pop();
        right = parseMultiplicativeExpression();
        return new ASTBinaryExpressionNode(left, right, op.readOperator());
    }

    return left;
}

ASTExpressionNode* Parser::parseMultiplicativeExpression() {
    ASTExpressionNode* left = parsePrimaryExpression();
    ASTExpressionNode* right = nullptr;

    auto op = m_lexar.peek();

    if (op.readOperator() == OperatorType::MULTIPLICATION || op.readOperator() == OperatorType::DIVISION) {
        m_lexar.pop();
        right = parsePrimaryExpression();
        return new ASTBinaryExpressionNode(left, right, op.readOperator());
    }

    return left;
}

ASTExpressionNode* Parser::parsePrimaryExpression() {
    Token token = m_lexar.peek();

    switch (token.readType()) {
        case CIPHTokenType::NUMBER:
            m_lexar.pop();
            return new ASTNumericLiteralNode(static_cast<int16_t>(std::stoi(token.readValue())));
        case CIPHTokenType::IDENTIFIER:
            return parseIdentifier();
        case CIPHTokenType::OPEN_PAREN: {
            m_lexar.pop();
            ASTExpressionNode* expression = parseAddativeExpression();
            if (m_lexar.expect(CIPHTokenType::CLOSE_PAREN) == false) {
                fmt::print("Expected closing parenthesis\n");
                return nullptr;
            }
            return expression;
        }
        case CIPHTokenType::OPERATOR: {
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

ASTLetNode* Parser::parseLetStatement() {
    m_lexar.pop();
    auto token = m_lexar.peek();
    if (token.readType() != CIPHTokenType::IDENTIFIER) {
        fmt::print(stderr, "Expected identifier\n");
        return nullptr;
    }

    std::string name = token.readValue();
    m_lexar.pop();

    if (m_lexar.popOperator(OperatorType::ASSIGNMENT) == false) {
        fmt::print(stderr, "Expected assignment operator\n");
        return nullptr;
    }

    ASTExpressionNode* expression = parseComparisonExpression();
    return new ASTLetNode(name, expression);
}

ASTIdentifierNode* Parser::parseIdentifier() {
    auto token = m_lexar.pop();
    if (token.readType() != CIPHTokenType::IDENTIFIER) {
        fmt::print(stderr, "Expected identifier\n");
        return nullptr;
    }

    std::string name = token.readValue();
    token = m_lexar.peek();

    if (token.readType() == CIPHTokenType::OPERATOR) {
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