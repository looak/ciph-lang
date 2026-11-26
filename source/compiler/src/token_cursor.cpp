#include <ciph/compiler/token_cursor.hpp>
#include <ciph/shared/shared_defines.hpp>

using namespace ciph;

Token
TokenCursor::pop() {
    Token token = m_tokens[m_position++];
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
    uint16_t peekPos = u16(m_position + offset);
    return m_tokens[peekPos];
}
