#include <ciph/compiler/token_cursor.hpp>

#include <cstddef>

using namespace ciph;

Token
TokenCursor::pop() {
    Token token = peek();
    if (m_position < m_tokens.size()) {
        ++m_position;
    }
    return token;
}

std::pair<bool, Token>
TokenCursor::popExpect(TokenType type) {
    Token token = pop();
    if (token.readType() != type)
        return {false, token};

    return {true, token};
}

std::pair<bool, Token>
TokenCursor::popOperator(OperatorType op) {
    Token token = pop();
    if (token.readType() != TokenType::OPERATOR)
        return {false, token};

    if (token.readOperatorValue() != op)
        return {false, token};

    return {true, token};
}

Token
TokenCursor::peek(uint16_t offset) const {
    const std::size_t index = std::size_t{m_position} + offset;
    if (index < m_tokens.size()) {
        return m_tokens[index];
    }

    // Outside the list: an empty token where the program ends.
    const Position end = m_tokens.empty() ? Position{} : m_tokens.back().readLocation().span.end;
    return Token("", TokenType::BEYOND_PROGRAM, end, 0, static_cast<uint32_t>(m_tokens.size()));
}
