#pragma once

#include <string>
#include <string_view>
#include <expected>

#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/lexer.hpp>
#include <ciph/compiler/token_cursor.hpp>

namespace ciph {

class ASTBaseNode;
class ASTScopeNode;
struct SourceRange;

class Parser {
public:
    // Builds the AST from already lexed tokens. The tokens, and the source
    // text they point into, must outlive the Parser and the AST it builds.
    explicit Parser(TokenCursor cursor);
    ~Parser() = default;

    Expect<ASTBaseNode*> parse();
    Expect<ASTBaseNode*> parseProgram();

private:
    // The range from token `first` to the token most recently popped.
    SourceRange rangeFrom(uint32_t first) const;

    Expect<ASTBaseNode*> parseStatement();
    // Pops the ';' that ends a simple statement and passes the statement on;
    // `what` names the statement in the error message. A statement that
    // failed to parse is passed on as it is, without looking for the ';'.
    Expect<ASTBaseNode*> expectSemicolon(Expect<ASTBaseNode*> statement, const char* what);
    // A call, `x++` or `x--` used as a statement. Any other expression is an
    // error, since its value would go unused.
    Expect<ASTBaseNode*> parseExpressionStatement();
    Expect<ASTBaseNode*> parseLetStatement();
    Expect<ASTBaseNode*> parseReturnStatement();
    // Defined in parser.cpp: every instantiation is made there.
    template <Expect<ASTBaseNode*> (Parser::*Next)(), OperatorType... Ops>
    Expect<ASTBaseNode*> parseBinaryLevel();
    Expect<ASTBaseNode*> parseAdditiveExpression();
    Expect<ASTBaseNode*> parseMultiplicativeExpression();
    Expect<ASTBaseNode*> parseUnaryExpression();
    Expect<ASTBaseNode*> parseIdentifier();
    Expect<ASTBaseNode*> parsePrimaryExpression();
    Expect<ASTBaseNode*> parseComparisonExpression();

    Expect<ASTBaseNode*> parseWhileStatement();
    Expect<ASTBaseNode*> parseWhileCondition();

    Expect<ASTBaseNode*> parseFunctionStatement();
    Expect<ASTBaseNode*> parseFunctionParameters();
    Expect<ASTBaseNode*> parseFunctionBody();

    // this assumption that I can reuse scope for while & function bodies might be flawed later...
    Expect<ASTBaseNode*> parseScopeNode(ASTScopeNode* scopeNode);

    TokenCursor m_cursor;
};

} // namespace ciph