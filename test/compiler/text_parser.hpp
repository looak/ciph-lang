#pragma once

// Test helper: lexes and parses source text in one go, so a test can write
// TextParser(code).parse(). The compiler itself keeps the two steps apart.

#include <string_view>
#include <vector>

#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/lexer.hpp>
#include <ciph/compiler/parser.hpp>
#include <ciph/compiler/token_cursor.hpp>

namespace ciph {

class ASTBaseNode;

// Holds the lexer, so the tokens stay alive for as long as the helper does.
// The text must outlive it too: tokens point into it.
class TextParser {
public:
    explicit TextParser(std::string_view text) : m_lexer(text) {
        auto [lexed, error] = m_lexer.lex();
        m_lexed = lexed;
        m_lexError = error;
    }

    // The AST, or the error that stopped lexing or parsing.
    Expect<ASTBaseNode*> parse() {
        if (!m_lexed) {
            return std::unexpected(Diagnostic{.code = m_lexError, .location = {}, .message = "Error while lexing input"});
        }
        Parser parser{TokenCursor{m_lexer.readTokens()}};
        return parser.parse();
    }

    const std::vector<Token>& readTokens() const { return m_lexer.readTokens(); }

private:
    Lexer m_lexer;
    bool m_lexed = false;
    ErrorCode m_lexError = ErrorCode::NO_ERR;
};

} // namespace ciph
