#include <ciph/compiler/parser.hpp>

#include <charconv>
#include <string_view>

#include <fmt/core.h>

#include <ciph/compiler/ast.hpp>

using namespace ciph;

namespace {

// The error result every parse function returns on failure. The message is
// added to the error code's default text.
std::unexpected<Diagnostic>
fail(ErrorCode code, Position position, std::string message) {
    return std::unexpected(Diagnostic{.code = code, .position = position, .additionalInfo = std::move(message)});
}

} // namespace

Parser::Parser(std::string_view input)
    : m_lexer(input) {
}

ParseExpect
Parser::parse() {
    auto [success, error_code] = m_lexer.lex();
    if (success == false) {
        return fail(error_code, {}, "Error while lexing input");
    }

    return parseProgram();
}

ParseExpect
Parser::parseProgram() {
    const uint32_t first = m_lexer.nextIndex(); // first token of this construct
    ASTProgramNode* program = new ASTProgramNode(SourceRange{first, first});
    auto token = m_lexer.peek();

    while (token.readType() != TokenType::END_OF_FILE) {
        auto statement = parseStatement();
        if (!statement) {
            return statement;
        }

        program->addStatement(*statement);
        token = m_lexer.peek();
    }

    // Everything up to, but not including, the end-of-file token. An empty
    // program keeps the one-token range it was created with.
    if (m_lexer.nextIndex() > first) {
        program->closeSourceRange(m_lexer.nextIndex() - 1);
    }
    return program;
}

SourceRange
Parser::rangeFrom(uint32_t first) const {
    return SourceRange{first, m_lexer.nextIndex() - 1};
}

ParseExpect
Parser::parseStatement() {
    // Simple statements end with a semicolon; block statements (while, fn) do
    // not. and_then only runs the semicolon check when the statement itself
    // parsed, otherwise the statement's own error is passed through.
    auto semicolonAfter = [this](const char* what) {
        return [this, what](ASTBaseNode* statement) { return expectSemicolon(statement, what); };
    };

    auto token = m_lexer.peek();
    switch (token.readType()) {
        case TokenType::RETURN:
            return parseReturnStatement().and_then(semicolonAfter("return statement"));
        case TokenType::LET:
            return parseLetStatement().and_then(semicolonAfter("let statement"));
        case TokenType::WHILE:
            return parseWhileStatement();
        case TokenType::FUNCTION:
            return parseFunctionStatement();
        case TokenType::SEMICOLON:
            return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION, token.readPosition(),
                        "Found ';' with no statement before it");
        default:
            return parseExpressionStatement().and_then(semicolonAfter("expression"));
    }
}

ParseExpect
Parser::parseExpressionStatement() {
    const Token token = m_lexer.peek();
    auto expression = parseComparisonExpression();
    if (!expression) {
        return expression;
    }

    // An expression can stand alone only when it does something itself: a
    // call, or `x++` / `x--`.
    if ((*expression)->readHasSideEffect()) {
        return expression;
    }

    return fail(ErrorCode::SYNTAX_ERROR_UNUSED_EXPRESSION, token.readPosition(),
                "The result of this expression is not used. Only a call, '++' or '--' can stand alone as a statement");
}

ParseExpect
Parser::expectSemicolon(ASTBaseNode* statement, const char* what) {
    const uint32_t next = m_lexer.nextIndex();
    auto [success, token] = m_lexer.popExpect(TokenType::SEMICOLON);
    if (success == false) {
        // Point just past the statement's last token, where the ';' belongs.
        // The token found instead is often on the next line.
        Position position = token.readPosition();
        if (next > 0) {
            const Token& last = m_lexer.readTokens()[next - 1];
            position = last.readPosition();
            position.column += last.readLength();
        }

        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON, position, fmt::format("Expected ';' after {}", what));
    }
    return statement;
}

ParseExpect
Parser::parseWhileCondition() {
    auto [success, expected_token] = m_lexer.popExpect(TokenType::OPEN_PAREN);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, expected_token.readPosition(), "Expecting a open parenthesis after while keyword which contains a expression");
    }

    auto condition = parseComparisonExpression();
    if (!condition) {
        Diagnostic error{   .code = ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION,
                            .position = expected_token.readPosition(),
                            .additionalInfo = "Expecting while loop to be defined with a expression.",
                            .nestedError = {condition.error()}};
        return std::unexpected(error);
    }

    auto [success2, expected_token2] = m_lexer.popExpect(TokenType::CLOSE_PAREN);
    if (success2 == false) {
        return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, expected_token2.readPosition(), "Expecting a close parenthesis after while keyword which contains a expression");
    }

    return condition;
}

ParseExpect
Parser::parseScopeNode(ASTScopeNode* scopeNode) {
    auto [success, expected_token] = m_lexer.popExpect(TokenType::OPEN_BRACE);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH, expected_token.readPosition(), "Expecting a open brace after keyword which contains a expression");
    }

    Token token = m_lexer.peek();
    while (token.readType() != TokenType::CLOSE_BRACE) {
        auto statement = parseStatement();
        if (!statement) {
            return statement;
        }

        scopeNode->addStatement(*statement);

        token = m_lexer.peek();
        if (token.readType() == TokenType::END_OF_FILE) {
            return fail(ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH, m_lexer.peek().readPosition(), "Expecting a close brace before end of file.");
        }
    }

    auto [success1, expected_token1] = m_lexer.popExpect(TokenType::CLOSE_BRACE);
    if (success1 == false) {
        return fail(ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH, expected_token1.readPosition(), "Expecting a close brace after open brance which contains statements");
    }

    scopeNode->closeSourceRange(m_lexer.nextIndex() - 1); // the closing brace
    return scopeNode;
}

ParseExpect
Parser::parseWhileStatement() {
    const uint32_t first = m_lexer.nextIndex(); // first token of this construct
    m_lexer.pop();

    auto condition = parseWhileCondition();
    if (!condition) {
        return condition;
    }

    ASTWhileNode* whileNode = new ASTWhileNode(SourceRange{first, first}, static_cast<ASTComparisonExpressionNode*>(*condition));

    auto body = parseScopeNode(whileNode);
    if (!body) {
        // create new error once we generalize while body into a scope parser
        return body;
    }

    return whileNode;
}

ParseExpect
Parser::parseReturnStatement() {
    const uint32_t first = m_lexer.nextIndex(); // first token of this construct
    m_lexer.pop();
    auto token = m_lexer.peek();
    ASTExpressionNode* expression = nullptr;
    if (token.readType() == TokenType::SEMICOLON) {
        // `return;` returns 0 as default
        expression = new ASTNumericLiteralNode(SourceRange{}, 0);
        expression->setOrigin(Origin::Synthesized);
    }
    else {
        auto result = parseComparisonExpression();
        if (!result) {
            return result;
        }
        expression = static_cast<ASTExpressionNode*>(*result);
    }

    return new ASTReturnNode(rangeFrom(first), expression);
}

ParseExpect
Parser::parseComparisonExpression() {
    const uint32_t first = m_lexer.nextIndex(); // first token of this construct
    auto left_result = parseAdditiveExpression();
    if (!left_result) {
        return left_result;
    }
    auto left = static_cast<ASTExpressionNode*>(*left_result);

    auto isComparison = [](const Token& token) {
        return token.readOperatorValue() == OperatorType::EQUAL || token.readOperatorValue() == OperatorType::NOT_EQUAL ||
               token.readOperatorValue() == OperatorType::LESS_THAN || token.readOperatorValue() == OperatorType::GREATER_THAN;
    };

    auto op = m_lexer.peek();
    if (isComparison(op)) {
        m_lexer.pop();

        auto right_result = parseAdditiveExpression();
        if (!right_result) {
            return right_result;
        }
        auto right = static_cast<ASTExpressionNode*>(*right_result);

        // A comparison takes one operator. A second one is reported here, at
        // that operator, rather than as a missing ';' by the caller.
        auto second = m_lexer.peek();
        if (isComparison(second)) {
            return fail(ErrorCode::SYNTAX_ERROR_CHAINED_COMPARISON, second.readPosition(),
                        "Comparisons cannot be chained, as in 'a == b == c'. Compare two values at a time");
        }

        return new ASTComparisonExpressionNode(rangeFrom(first), left, right, op.readOperatorValue());
    }

    return left;
}

// One precedence level of left-associative binary operators: parses `Next`,
// then folds `op Next` into the left side for as long as the next token is
// one of `Ops`. So `a - b - c` becomes `(a - b) - c`.
template <ParseExpect (Parser::*Next)(), OperatorType... Ops>
ParseExpect
Parser::parseBinaryLevel() {
    const uint32_t first = m_lexer.nextIndex(); // first token of this construct
    auto left_result = (this->*Next)();
    if (!left_result) {
        return left_result;
    }
    auto left = static_cast<ASTExpressionNode*>(*left_result);

    auto op = m_lexer.peek();
    while (((op.readOperatorValue() == Ops) || ...)) {
        m_lexer.pop();

        auto right_result = (this->*Next)();
        if (!right_result) {
            return right_result;
        }
        auto right = static_cast<ASTExpressionNode*>(*right_result);

        left = new ASTBinaryExpressionNode(rangeFrom(first), left, right, op.readOperatorValue());
        op = m_lexer.peek();
    }

    return left;
}

ParseExpect
Parser::parseAdditiveExpression() {
    return parseBinaryLevel<&Parser::parseMultiplicativeExpression, OperatorType::ADDITION, OperatorType::SUBTRACTION>();
}

ParseExpect
Parser::parseMultiplicativeExpression() {
    return parseBinaryLevel<&Parser::parsePrimaryExpression, OperatorType::MULTIPLICATION, OperatorType::DIVISION>();
}

ParseExpect
Parser::parsePrimaryExpression() {
    const uint32_t first = m_lexer.nextIndex(); // first token of this construct
    Token token = m_lexer.peek();

    switch (token.readType()) {
        case TokenType::NUMBER: {
            m_lexer.pop();
            // Reads the digits straight from the source text. A literal too
            // large for an int leaves the value at 0.
            const std::string_view digits = token.readStringValue();
            int numericValue = 0;
            std::from_chars(digits.data(), digits.data() + digits.size(), numericValue);
            return new ASTNumericLiteralNode(rangeFrom(first), static_cast<int16_t>(numericValue));
        }
        case TokenType::IDENTIFIER:
            return parseIdentifier();
        case TokenType::OPEN_PAREN: {
            m_lexer.pop();

            auto expression = parseAdditiveExpression();
            if (!expression) {
                return expression;
            }

            auto [success, expected_token] = m_lexer.popExpect(TokenType::CLOSE_PAREN);
            if (success == false) {
                return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, expected_token.readPosition(), "Missmatch between open and close parenthesis in expression.");
            }

            return expression;
        }

        default: {
            // Operator tokens carry no text, so only name what was found when
            // there is something to show.
            std::string message = "Expected an expression";
            if (token.readType() == TokenType::END_OF_FILE) {
                message += " before the end of the file";
            }
            else if (!token.readStringValue().empty()) {
                message += fmt::format(", found '{}'", token.readStringValue());
            }
            return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION, token.readPosition(), std::move(message));
        }
    }
}

ParseExpect
Parser::parseLetStatement() {
    const uint32_t first = m_lexer.nextIndex(); // first token of this construct
    m_lexer.pop();
    auto [success, token] = m_lexer.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER, token.readPosition(), "Expected identifier after let keyword");
    }

    auto name = token.readStringValue();

    auto [op_success, op_token] = m_lexer.popOperator(OperatorType::ASSIGNMENT);
    if (op_success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_OPERATOR, op_token.readPosition(), fmt::format("Expected assignment operator after identifier {} in let statement", name));
    }

    auto expression = parseComparisonExpression();
    if (!expression) {
        return expression;
    }

    auto range = rangeFrom(first);
    range.focus = token.readIndex();
    return new ASTLetNode(range, name, static_cast<ASTExpressionNode*>(*expression));
}

ParseExpect
Parser::parseIdentifier() {
    const uint32_t first = m_lexer.nextIndex(); // first token of this construct
    auto [success, token] = m_lexer.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER, token.readPosition(), "Expected identifier in expression");
    }

    std::string_view name = token.readStringValue();
    token = m_lexer.peek();

    if (token.readType() == TokenType::OPEN_PAREN) {
        m_lexer.pop(); // pop open paren
        // handle arguments later
        auto [close_success, result] = m_lexer.popExpect(TokenType::CLOSE_PAREN);
        if (close_success == false) {
            return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, result.readPosition(), "Expected closing parenthesis for function call");
        }
        return new ASTCallNode(rangeFrom(first), name);
    }
    else if (token.readOperatorValue() == OperatorType::INCREMENT) {
        m_lexer.pop();
        return new ASTIdentifierNode(rangeFrom(first), name, IncDec::PostIncrement);
    }
    else if (token.readOperatorValue() == OperatorType::DECREMENT) {
        m_lexer.pop();
        return new ASTIdentifierNode(rangeFrom(first), name, IncDec::PostDecrement);
    }

    return new ASTIdentifierNode(rangeFrom(first), name);
}

ParseExpect
Parser::parseFunctionStatement() {
    const uint32_t first = m_lexer.nextIndex(); // first token of this construct
    m_lexer.pop();
    auto [success, token] = m_lexer.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER, token.readPosition(), "Expected function identifier after fn keyword");
    }
    auto name = token.readStringValue();
    SourceRange range{first, first};
    range.focus = token.readIndex(); // the name, not the `fn` keyword
    auto functionNode = new ASTFunctionNode(range, name);

    m_lexer.popExpect(TokenType::OPEN_PAREN);
    // handle parameters later
    m_lexer.popExpect(TokenType::CLOSE_PAREN);

    auto body = parseScopeNode(functionNode);
    if (!body) {
        return body;
    }

    const auto& statements = functionNode->readStatements();
    bool hasReturn = !statements.empty() && statements.back()->readType() == ASTNodeType::RETURN;
    if (hasReturn == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_RETURN_STATEMENT, token.readPosition(), "Expected function to have a return statement");
    }

    return functionNode;
}
