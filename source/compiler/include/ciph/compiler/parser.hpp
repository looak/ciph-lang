#pragma once

#include <string>
#include <expected>

#include <ciph/compiler/error_defines.hpp>
#include <ciph/compiler/lexar.hpp>

namespace ciph {

class ASTBaseNode;
class ASTScopeNode;
struct SourceRange;
struct ParserError {
    ErrorCode code;
    Position position;
    // this string is supposed to be populated when the error is created, if needed.
    // default error messages are provided with the error code, and this will be appended to those.
    std::string additionalInfo;
    std::vector<ParserError> nestedError = {};
};

using ParseExpect = std::expected<ASTBaseNode*, ParserError>;

class Parser {
public:
    explicit Parser(const std::string& input);
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
    ParseExpect parseLetStatement();
    ParseExpect parseReturnStatement();
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