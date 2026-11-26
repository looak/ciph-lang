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
    explicit TextParser(std::string_view text) : m_lexer(text) { m_lexer.lex(); }

    // The AST, or the error that stopped lexing or parsing. For a lexing
    // error that is the lexer's first diagnostic.
    Expect<ASTBaseNode*> parse() {
        if (m_lexer.hasErrors()) {
            return std::unexpected(m_lexer.readDiagnostics().front());
        }
        Parser parser{TokenCursor{m_lexer.readTokens()}};
        return parser.parse();
    }

    const std::vector<Token>& readTokens() const { return m_lexer.readTokens(); }

private:
    Lexer m_lexer;
};

} // namespace ciph
