#include "parser.hpp"

#include <fmt/core.h>

#include "ast.hpp"

using namespace ciph;

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
        case TokenType::FUNCTION:
            return parseFunctionStatement();
        case TokenType::IDENTIFIER:

        default:
            return parseComparisonExpression();
    }
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseWhileCondition() {
    auto [success, expected_token] = m_lexar.popExpect(TokenType::OPEN_PAREN);
    if (success == false) {
        ParserError error{
            .code = ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH,
            .position = expected_token.readPosition(),
            .additionalInfo = "Expecting a open parenthesis after while keyword which contains a expression"};
        return error;
    }

    ASTExpressionNode* condition = nullptr;
    auto condition_result = parseComparisonExpression();
    if (auto condition_ptr = std::get_if<ASTBaseNode*>(&condition_result)) {
        condition = static_cast<ASTExpressionNode*>(*condition_ptr);
    }
    else {
        ParserError innerError = std::get<ParserError>(condition_result);
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION,
                          .position = expected_token.readPosition(),
                          .additionalInfo = "Expecting while loop to be defined with a expression.",
                          .nestedError = {innerError}};
        return error;
    }

    auto [success2, expected_token2] = m_lexar.popExpect(TokenType::CLOSE_PAREN);
    if (success2 == false) {
        ParserError error{
            .code = ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH,
            .position = expected_token2.readPosition(),
            .additionalInfo = "Expecting a close parenthesis after while keyword which contains a expression"};
        return error;
    }

    return condition;
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseScopeNode(ASTScopeNode* scopeNode) {
    auto [success, expected_token] = m_lexar.popExpect(TokenType::OPEN_BRACE);
    if (success == false) {
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH,
                          .position = expected_token.readPosition(),
                          .additionalInfo = "Expecting a open brace after keyword which contains a expression"};
        return error;
    }

    Token token = m_lexar.peek();
    while (token.readType() != TokenType::CLOSE_BRACE) {
        ASTBaseNode* statement = nullptr;
        auto statement_result = parseStatement();
        if (auto statement_ptr = std::get_if<ASTBaseNode*>(&statement_result)) {
            statement = *statement_ptr;
        }
        else {
            return std::get<ParserError>(statement_result);
        }

        scopeNode->addStatement(statement);

        token = m_lexar.peek();
        if (token.readType() == TokenType::END_OF_FILE) {
            ParserError error{.code = ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH,
                              .position = m_lexar.peek().readPosition(),
                              .additionalInfo = "Expecting a close brace before end of file."};
        }
    }

    auto [success1, expected_token1] = m_lexar.popExpect(TokenType::CLOSE_BRACE);
    if (success1 == false) {
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH,
                          .position = expected_token1.readPosition(),
                          .additionalInfo = "Expecting a close brace after open brance which contains statements"};
        return error;
    }

    return scopeNode;
}

std::variant<ParserError, ASTBaseNode*>
Parser::parseWhileStatement() {
    m_lexar.pop();

    ASTComparisonExpressionNode* condition = nullptr;
    auto condition_result = parseWhileCondition();
    if (auto condition_ptr = std::get_if<ASTBaseNode*>(&condition_result)) {
        condition = static_cast<ASTComparisonExpressionNode*>(*condition_ptr);
    }
    else {
        return std::get<ParserError>(condition_result);
    }

    ASTWhileNode* whileNode = new ASTWhileNode(condition);

    auto body_result = parseScopeNode(whileNode);
    if (auto body_ptr = std::get_if<ParserError>(&body_result)) {
        // create new error once we generalize while body into a scope parser
        return body_result;
    }

    return whileNode;
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
                ParserError error{.code = ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH,
                                  .position = expected_token.readPosition(),
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
        ParserError error{  .code = ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER,
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
        ParserError error{  .code = ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER,
                            .position = token.readPosition(),
                            .additionalInfo = "Expected identifier in expression"};
        return error;
    }

    std::string name = token.readValue();
    token = m_lexar.peek();

    if (token.readType() == TokenType::OPEN_PAREN) {
        m_lexar.pop(); // pop open paren
        // handle arguments later
        auto [success, result] = m_lexar.popExpect(TokenType::CLOSE_PAREN);
        if (success == false) {
            ParserError error{  .code = ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH,
                                .position = result.readPosition(),
                                .additionalInfo = "Expected closing parenthesis for function call"};
            return error;
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

std::variant<ParserError, ASTBaseNode*>
Parser::parseFunctionStatement() {
    m_lexar.pop();
    auto [success, token] = m_lexar.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        ParserError error{  .code = ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER,
                            .position = token.readPosition(),
                            .additionalInfo = "Expected function identifier after fn keyword"};
        return error;
    }
    auto name = token.readValue();
    auto functionNode = new ASTFunctionNode(name);

    m_lexar.popExpect(TokenType::OPEN_PAREN);
    // handle parameters later
    m_lexar.popExpect(TokenType::CLOSE_PAREN);

    parseScopeNode(functionNode);
    
    bool hasReturn = functionNode->readStatements().back()->readType() == ASTNodeType::RETURN;
    if (hasReturn == false) {
        ParserError error{.code = ErrorCode::SYNTAX_ERROR_EXPECTED_RETURN_STATEMENT,
                          .position = token.readPosition(),
                          .additionalInfo = "Expected function to have a return statement"};
        return error;
    }

    return functionNode;
}
