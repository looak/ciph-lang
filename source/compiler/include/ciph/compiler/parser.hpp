#pragma once

#include <string>
#include <string_view>
#include <expected>

#include <ciph/compiler/error_defines.hpp>
#include <ciph/compiler/lexar.hpp>

namespace ciph {

class ASTBaseNode;
class ASTScopeNode;
struct SourceRange;

using ParseExpect = std::expected<ASTBaseNode*, Diagnostic>;

class Parser {
public:
    // The parser and its lexer keep string_views into `input` (tokens point
    // at their text), so the characters must outlive the Parser. A string
    // literal or a CompileUnit's source is fine; a temporary std::string
    // is not.
    explicit Parser(std::string_view input);
    ~Parser() = default;

    ParseExpect parse();
    ParseExpect parseProgram();

    // The tokens the program was parsed from. An AST node's SourceRange holds
    // indices into this list.
    const std::vector<Token>& readTokens() const { return m_lexar.readTokens(); }

private:
    // The range from token `first` to the token most recently popped.
    SourceRange rangeFrom(uint32_t first) const;

    ParseExpect parseStatement();
    // Pops the ';' that ends a simple statement and passes the statement on;
    // `what` names the statement in the error message.
    ParseExpect expectSemicolon(ASTBaseNode* statement, const char* what);
    // A call, `x++` or `x--` used as a statement. Any other expression is an
    // error, since its value would go unused.
    ParseExpect parseExpressionStatement();
    ParseExpect parseLetStatement();
    ParseExpect parseReturnStatement();
    // Defined in parser.cpp: every instantiation is made there.
    template <ParseExpect (Parser::*Next)(), OperatorType... Ops>
    ParseExpect parseBinaryLevel();
    ParseExpect parseAddativeExpression();
    ParseExpect parseMultiplicativeExpression();
    ParseExpect parseIdentifier();
    ParseExpect parsePrimaryExpression();
    ParseExpect parseComparisonExpression();

    ParseExpect parseWhileStatement();
    ParseExpect parseWhileCondition();

    ParseExpect parseFunctionStatement();
    ParseExpect parseFunctionParameters();
    ParseExpect parseFunctionBody();

    // this assumption that I can reuse scope for while & function bodies might be flawed later...
    ParseExpect parseScopeNode(ASTScopeNode* scopeNode);

    Lexar m_lexar;
};

} // namespace ciph