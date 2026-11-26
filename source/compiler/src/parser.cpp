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
fail(ErrorCode code, Location location, std::string message) {
    return std::unexpected(Diagnostic{.code = code, .location = location, .message = std::move(message)});
}

} // namespace

Parser::Parser(TokenCursor cursor)
    : m_cursor(cursor) {
}

Expect<ASTBaseNode*>
Parser::parse() {
    return parseProgram();
}

Expect<ASTBaseNode*>
Parser::parseProgram() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    ASTProgramNode* program = new ASTProgramNode(SourceRange{first, first});
    auto token = m_cursor.peek();

    while (token.readType() != TokenType::END_OF_FILE) {
        auto statement = parseStatement();
        if (!statement) {
            return statement;
        }

        program->addStatement(*statement);
        token = m_cursor.peek();
    }

    // Everything up to, but not including, the end-of-file token. An empty
    // program keeps the one-token range it was created with.
    if (m_cursor.nextIndex() > first) {
        program->closeSourceRange(m_cursor.nextIndex() - 1);
    }
    return program;
}

SourceRange
Parser::rangeFrom(uint32_t first) const {
    return SourceRange{first, m_cursor.nextIndex() - 1};
}

Expect<ASTBaseNode*>
Parser::parseStatement() {
    // Simple statements end with a semicolon; block statements (while, fn) do
    // not.
    auto token = m_cursor.peek();
    switch (token.readType()) {
        case TokenType::RETURN:
            return expectSemicolon(parseReturnStatement(), "return statement");
        case TokenType::LET:
            return expectSemicolon(parseLetStatement(), "let statement");
        case TokenType::WHILE:
            return parseWhileStatement();
        case TokenType::FUNCTION:
            return parseFunctionStatement();
        case TokenType::SEMICOLON:
            return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION, token.readLocation(),
                        "Found ';' with no statement before it");
        default:
            return expectSemicolon(parseExpressionStatement(), "expression");
    }
}

Expect<ASTBaseNode*>
Parser::parseExpressionStatement() {
    const Token token = m_cursor.peek();
    auto expression = parseComparisonExpression();
    if (!expression) {
        return expression;
    }

    // An expression can stand alone only when it does something itself: a
    // call, or `x++` / `x--`.
    if ((*expression)->readHasSideEffect()) {
        return expression;
    }

    return fail(ErrorCode::SYNTAX_ERROR_UNUSED_EXPRESSION, token.readLocation(),
                "The result of this expression is not used. Only a call, '++' or '--' can stand alone as a statement");
}

Expect<ASTBaseNode*>
Parser::expectSemicolon(Expect<ASTBaseNode*> statement, const char* what) {
    if (!statement) {
        return statement;
    }

    const uint32_t next = m_cursor.nextIndex();
    auto [success, token] = m_cursor.popExpect(TokenType::SEMICOLON);
    if (success == false) {
        // Point just past the statement's last token, where the ';' belongs.
        // The token found instead is often on the next line. A point, not a
        // character, so the span is empty.
        Position position = token.readPosition();
        if (next > 0) {
            position = m_cursor.readTokens()[next - 1].readLocation().span.end;
        }

        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON, Location{.span = {.start = position, .end = position}},
                    fmt::format("Expected ';' after {}", what));
    }
    return statement;
}

Expect<ASTBaseNode*>
Parser::parseWhileCondition() {
    auto [success, expected_token] = m_cursor.popExpect(TokenType::OPEN_PAREN);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, expected_token.readLocation(), "Expecting a open parenthesis after while keyword which contains a expression");
    }

    auto condition = parseComparisonExpression();
    if (!condition) {
        return condition;
    }

    auto [success2, expected_token2] = m_cursor.popExpect(TokenType::CLOSE_PAREN);
    if (success2 == false) {
        return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, expected_token2.readLocation(), "Expecting a close parenthesis after while keyword which contains a expression");
    }

    return condition;
}

Expect<ASTBaseNode*>
Parser::parseScopeNode(ASTScopeNode* scopeNode) {
    auto [success, expected_token] = m_cursor.popExpect(TokenType::OPEN_BRACE);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH, expected_token.readLocation(), "Expecting a open brace after keyword which contains a expression");
    }

    Token token = m_cursor.peek();
    while (token.readType() != TokenType::CLOSE_BRACE) {
        auto statement = parseStatement();
        if (!statement) {
            return statement;
        }

        scopeNode->addStatement(*statement);

        token = m_cursor.peek();
        if (token.readType() == TokenType::END_OF_FILE) {
            return fail(ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH, m_cursor.peek().readLocation(), "Expecting a close brace before end of file.");
        }
    }

    auto [success1, expected_token1] = m_cursor.popExpect(TokenType::CLOSE_BRACE);
    if (success1 == false) {
        return fail(ErrorCode::SYNTAX_ERROR_BRACE_MISSMATCH, expected_token1.readLocation(), "Expecting a close brace after open brance which contains statements");
    }

    scopeNode->closeSourceRange(m_cursor.nextIndex() - 1); // the closing brace
    return scopeNode;
}

Expect<ASTBaseNode*>
Parser::parseWhileStatement() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    m_cursor.pop();

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

Expect<ASTBaseNode*>
Parser::parseReturnStatement() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    m_cursor.pop();
    auto token = m_cursor.peek();
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

Expect<ASTBaseNode*>
Parser::parseComparisonExpression() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    auto left_result = parseAdditiveExpression();
    if (!left_result) {
        return left_result;
    }
    auto left = static_cast<ASTExpressionNode*>(*left_result);

    auto isComparison = [](const Token& token) {
        return token.readOperatorValue() == OperatorType::EQUAL || token.readOperatorValue() == OperatorType::NOT_EQUAL ||
               token.readOperatorValue() == OperatorType::LESS_THAN || token.readOperatorValue() == OperatorType::GREATER_THAN;
    };

    auto op = m_cursor.peek();
    if (isComparison(op)) {
        m_cursor.pop();

        auto right_result = parseAdditiveExpression();
        if (!right_result) {
            return right_result;
        }
        auto right = static_cast<ASTExpressionNode*>(*right_result);

        // A comparison takes one operator. A second one is reported here, at
        // that operator, rather than as a missing ';' by the caller.
        auto second = m_cursor.peek();
        if (isComparison(second)) {
            return fail(ErrorCode::SYNTAX_ERROR_CHAINED_COMPARISON, second.readLocation(),
                        "Comparisons cannot be chained, as in 'a == b == c'. Compare two values at a time");
        }

        return new ASTComparisonExpressionNode(rangeFrom(first), left, right, op.readOperatorValue());
    }

    return left;
}

// One precedence level of left-associative binary operators: parses `Next`,
// then folds `op Next` into the left side for as long as the next token is
// one of `Ops`. So `a - b - c` becomes `(a - b) - c`.
template <Expect<ASTBaseNode*> (Parser::*Next)(), OperatorType... Ops>
Expect<ASTBaseNode*>
Parser::parseBinaryLevel() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    auto left_result = (this->*Next)();
    if (!left_result) {
        return left_result;
    }
    auto left = static_cast<ASTExpressionNode*>(*left_result);

    auto op = m_cursor.peek();
    while (((op.readOperatorValue() == Ops) || ...)) {
        m_cursor.pop();

        auto right_result = (this->*Next)();
        if (!right_result) {
            return right_result;
        }
        auto right = static_cast<ASTExpressionNode*>(*right_result);

        left = new ASTBinaryExpressionNode(rangeFrom(first), left, right, op.readOperatorValue());
        op = m_cursor.peek();
    }

    return left;
}

Expect<ASTBaseNode*>
Parser::parseAdditiveExpression() {
    return parseBinaryLevel<&Parser::parseMultiplicativeExpression, OperatorType::ADDITION, OperatorType::SUBTRACTION>();
}

Expect<ASTBaseNode*>
Parser::parseMultiplicativeExpression() {
    return parseBinaryLevel<&Parser::parseUnaryExpression, OperatorType::MULTIPLICATION, OperatorType::DIVISION>();
}

// `-x`. The operand is itself a unary expression, so a minus can repeat:
// `- -x` is `-(-x)`. Sitting below the multiplicative level makes it bind
// tighter than `*` and `/`: `-2 * 3` is `(-2) * 3`.
Expect<ASTBaseNode*>
Parser::parseUnaryExpression() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    const Token op = m_cursor.peek();
    if (op.readOperatorValue() != OperatorType::SUBTRACTION) {
        return parsePrimaryExpression();
    }
    m_cursor.pop();

    auto operand = parseUnaryExpression();
    if (!operand) {
        return operand;
    }

    return new ASTUnaryExpressionNode(rangeFrom(first), *operand, op.readOperatorValue());
}

Expect<ASTBaseNode*>
Parser::parsePrimaryExpression() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    Token token = m_cursor.peek();

    switch (token.readType()) {
        case TokenType::NUMBER: {
            m_cursor.pop();
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
            m_cursor.pop();

            auto expression = parseComparisonExpression();            
            if (!expression) {
                return expression;
            }

            auto [success, expected_token] = m_cursor.popExpect(TokenType::CLOSE_PAREN);
            if (success == false) {
                return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, expected_token.readLocation(), "Missmatch between open and close parenthesis in expression.");
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
            return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION, token.readLocation(), std::move(message));
        }
    }
}

Expect<ASTBaseNode*>
Parser::parseLetStatement() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    m_cursor.pop();
    auto [success, token] = m_cursor.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER, token.readLocation(), "Expected identifier after let keyword");
    }

    auto name = token.readStringValue();

    auto [op_success, op_token] = m_cursor.popOperator(OperatorType::ASSIGNMENT);
    if (op_success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_OPERATOR, op_token.readLocation(), fmt::format("Expected assignment operator after identifier {} in let statement", name));
    }

    auto expression = parseComparisonExpression();
    if (!expression) {
        return expression;
    }

    auto range = rangeFrom(first);
    range.focus = token.readIndex();
    return new ASTLetNode(range, name, static_cast<ASTExpressionNode*>(*expression));
}

Expect<ASTBaseNode*>
Parser::parseIdentifier() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    auto [success, token] = m_cursor.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER, token.readLocation(), "Expected identifier in expression");
    }

    std::string_view name = token.readStringValue();
    token = m_cursor.peek();

    if (token.readType() == TokenType::OPEN_PAREN) {
        m_cursor.pop(); // pop open paren
        // handle arguments later
        auto [close_success, result] = m_cursor.popExpect(TokenType::CLOSE_PAREN);
        if (close_success == false) {
            return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, result.readLocation(), "Expected closing parenthesis for function call");
        }
        return new ASTCallNode(rangeFrom(first), name);
    }
    else if (token.readOperatorValue() == OperatorType::INCREMENT) {
        m_cursor.pop();
        return new ASTIdentifierNode(rangeFrom(first), name, IncDec::PostIncrement);
    }
    else if (token.readOperatorValue() == OperatorType::DECREMENT) {
        m_cursor.pop();
        return new ASTIdentifierNode(rangeFrom(first), name, IncDec::PostDecrement);
    }

    return new ASTIdentifierNode(rangeFrom(first), name);
}

Expect<ASTBaseNode*>
Parser::parseFunctionStatement() {
    const uint32_t first = m_cursor.nextIndex(); // first token of this construct
    m_cursor.pop();
    auto [success, token] = m_cursor.popExpect(TokenType::IDENTIFIER);
    if (success == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER, token.readLocation(), "Expected function identifier after fn keyword");
    }
    auto name = token.readStringValue();
    SourceRange range{first, first};
    range.focus = token.readIndex(); // the name, not the `fn` keyword
    auto functionNode = new ASTFunctionNode(range, name);

    auto [open, openToken] = m_cursor.popExpect(TokenType::OPEN_PAREN);
    if (open == false) {
        return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, openToken.readLocation(),
                    "Expected '(' after the function name");
    }
    // handle parameters later
    auto [close, closeToken] = m_cursor.popExpect(TokenType::CLOSE_PAREN);
    if (close == false) {
        return fail(ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH, closeToken.readLocation(),
                    "Expected ')': functions take no parameters yet");
    }

    auto body = parseScopeNode(functionNode);
    if (!body) {
        return body;
    }

    const auto& statements = functionNode->readStatements();
    bool hasReturn = !statements.empty() && statements.back()->readType() == ASTNodeType::RETURN;
    if (hasReturn == false) {
        return fail(ErrorCode::SYNTAX_ERROR_EXPECTED_RETURN_STATEMENT, token.readLocation(), "Expected function to have a return statement");
    }

    return functionNode;
}
