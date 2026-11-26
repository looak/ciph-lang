#pragma once

#include <cstdint>
#include <span>
#include <utility>

#include <ciph/compiler/lexer.hpp>
#include <ciph/compiler/lexer_defines.hpp>

namespace ciph {

// Walks a list of tokens for the parser: pop takes the next one, peek looks
// ahead. The list ends with END_OF_FILE. The cursor only views the tokens, so
// whoever made them (the Lexer) must keep them alive.
class TokenCursor {
public:
    explicit TokenCursor(std::span<const Token> tokens) : m_tokens(tokens) {}

    // The next token, moving past it. Past END_OF_FILE this returns a
    // BEYOND_PROGRAM token and stays where it is.
    Token pop();

    // Pops the next token. The bool says whether it has the expected type;
    // the token is returned either way.
    std::pair<bool, Token> popExpect(TokenType type);

    // Pops the next token. The bool says whether it is the operator `op`; the
    // token is returned either way.
    std::pair<bool, Token> popOperator(OperatorType op);

    // The token `offset` places ahead, without moving. 0 is the next token.
    // Beyond the list this is a BEYOND_PROGRAM token.
    Token peek(uint16_t offset = 0) const;

    // Index, into readTokens(), of the token the next pop() returns. The
    // parser uses it to record which tokens an AST node was built from.
    uint32_t nextIndex() const { return m_position; }

    std::span<const Token> readTokens() const { return m_tokens; }

private:
    std::span<const Token> m_tokens;
    uint32_t m_position = 0;
};

} // namespace ciph
