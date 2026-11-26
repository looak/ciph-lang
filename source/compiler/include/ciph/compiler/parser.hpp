#pragma once

#include <string>
#include <string_view>
#include <expected>

#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/lexer.hpp>

namespace ciph {

class ASTBaseNode;
class ASTScopeNode;
struct SourceRange;

class Parser {
public:
    // The parser and its lexer keep string_views into `input` (tokens point
    // at their text), so the characters must outlive the Parser. A string
    // literal or a CompileUnit's source is fine; a temporary std::string
    // is not.
    explicit Parser(std::string_view input);
    ~Parser() = default;

    Expect<ASTBaseNode*> parse();
    Expect<ASTBaseNode*> parseProgram();

    // The tokens the program was parsed from. An AST node's SourceRange holds
    // indices into this list.
    const std::vector<Token>& readTokens() const { return m_lexer.readTokens(); }

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

    Lexer m_lexer;
};

} // namespace ciph