#include "lexar.hpp"

#include <shared_defines.hpp>

#include "lexar_defines.hpp"

Token::Token(const std::string& value, TokenType type, Position position)
    : m_value(value)
    , m_type(type)
    , m_position(position) {
}

Token::Token(const OperatorType& value, TokenType type, Position position)
    : m_value(value)
    , m_type(type)
    , m_position(position) {
}

Token::Token(const std::variant<std::string, OperatorType>& value, TokenType type, Position position)
    : m_value(value)
    , m_type(type)
    , m_position(position) {
}

Token
Token::operator=(const Token& other) {
    m_value = other.m_value;
    m_type = other.m_type;
    m_position = other.m_position;
    return *this;
}

std::string
Token::readValue() const {
    return std::get<std::string>(m_value);
}

OperatorType
Token::readOperator() const {
    if (m_type != TokenType::OPERATOR)
        return OperatorType::UNKNOWN;

    return std::get<OperatorType>(m_value);
}

TokenType
Token::readType() const {
    return m_type;
}

bool
Token::hasValue() const {
    return m_type != TokenType::UNKNOWN;
}

Position
Token::readPosition() const {
    return m_position;
}

Lexar::Lexar(const std::string& input)
    : m_input(input)
    , m_position(0)
    , m_tokens()
    , m_performedAnalysis(false) {
}

void
Lexar::push(std::variant<std::string, OperatorType> value, TokenType type, Position& position) {
    m_tokens.emplace_back(value, type, position);
    position.column++;
}

void
Lexar::skipWhiteSpaces(Position& cursorPos) {
    bool done = false;
    do {
        if (m_input[m_position] == '\n') {
            cursorPos.line++;
            cursorPos.column = 1;
            m_position++;
        }
        else if (m_input[m_position] == '\t') {
            cursorPos.column++; // maybe should do 4
            m_position++;
        }
        else if (m_input[m_position] == ' ') {
            cursorPos.column++;
            m_position++;
        }
        else {
            done = true;
        }

    } while (!done);
}

LexarResult
Lexar::lex() {
    if (m_performedAnalysis)
        return {false, ErrorCode::LEXAR_PERFORMED_ANALYSIS};
    Position cursorPosition = {1, 1};

    while (m_position < m_input.size()) {
        TokenType type = TokenType::UNKNOWN;
        std::string value = "";
        skipWhiteSpaces(cursorPosition);
        char cursor = popNextChar();
        if (cursor == '(') {
            push("(", TokenType::OPEN_PAREN, cursorPosition);
            continue;
        }
        else if (cursor == ')') {
            push(")", TokenType::CLOSE_PAREN, cursorPosition);
            continue;
        }
        else if (cursor == '[') {
            push("[", TokenType::OPEN_BRACKET, cursorPosition);
            continue;
        }
        else if (cursor == ']') {
            push("]", TokenType::CLOSE_BRACKET, cursorPosition);
            continue;
        }
        else if (cursor == '{') {
            push("{", TokenType::OPEN_BRACE, cursorPosition);
            continue;
        }
        else if (cursor == '}') {
            push("}", TokenType::CLOSE_BRACE, cursorPosition);
            continue;
        }

        if (isdigit(cursor)) {
            uint32_t start = cursorPosition.column;
            do {
                value += cursor;
                type = TokenType::NUMBER;

                cursor = popNextChar();
                cursorPosition.column++;

            } while (isdigit(cursor));

            // move position back one step since we read past the last digit.
            m_position--;
            cursorPosition.column--;


            // idea here is to set the cursor to the beginning of the digit, so that when it's referenced later we're
            // pointing the user to the beginning of the digit.
            uint32_t difference = cursorPosition.column - start;
            cursorPosition.column -= difference;
            push(value, type, cursorPosition);
            cursorPosition.column += difference;
            continue;
        }

        if (isalpha(cursor)) {
            uint32_t start = cursorPosition.column;
            do {
                value += cursor;
                type = TokenType::IDENTIFIER;

                cursor = popNextChar();
                cursorPosition.column++;

            } while (isalpha(cursor));

            m_position--;
            cursorPosition.column--;

            if (s_keywords.find(value) != s_keywords.end()) {
                type = s_keywords[value];
            }

            // idea here is to set the cursor to the beginning of the digit, so that when it's referenced later we're
            // pointing the user to the beginning of the digit.
            uint32_t difference = cursorPosition.column - start;
            cursorPosition.column -= difference;
            push(value, type, cursorPosition);
            cursorPosition.column += difference;
            continue;
        }

        if (identifyOperator(cursor, cursorPosition) == true) {
            continue;
        }

        // if we reach this without hitting a continue, we have an unknown character
        return {false, ErrorCode::LEXAR_UNKNOWN_CHARACTER};
    }

    // reset position, we'll reuse this variable when we pop & peek the tokens in our vector.
    m_position = 0;
    m_performedAnalysis = true;
    push("eof", TokenType::END_OF_FILE, cursorPosition);
    return {true, ErrorCode::NO_ERROR};
}

std::pair<bool, Token>
Lexar::popOperator(OperatorType op) {
    Token token = pop();
    if (token.readType() != TokenType::OPERATOR)
        return {false, token};
    if (token.readOperator() != op)
        return {false, token};

    return {true, token};
}

std::pair<bool, Token>
Lexar::popExpect(TokenType type) {
    Token token = pop();
    if (token.readType() != type)
        return {false, token};

    return {true, token};
}

Token
Lexar::peek(uint16_t offset) const {
    uint16_t peekPos = u16(m_position + offset);
    return m_tokens[peekPos];
}

Token
Lexar::pop() {
    return m_tokens[m_position++];
}

char
Lexar::popNextChar() {
    return m_input[m_position++];
}

char
Lexar::peekNextChar() {
    if (m_position >= m_input.size())
        return '\0';

    return m_input[m_position];
}

bool
Lexar::isOperator(char cursor) {
    if (cursor == '\0')
        return false;

    for (auto& op : s_operators) {
        if (op.first == cursor)
            return true;
    }
    return false;
}

bool
Lexar::identifyOperator(char cursor, Position& cursorPos) {
    for (auto& op : s_operators) {
        if (op.first == cursor) {
            char peek = peekNextChar();

            if (isOperator(peek) == false) {
                push(op.second[0].second, TokenType::OPERATOR, cursorPos);
                return true;
            }

            for (auto& sub_op : op.second) {
                if (sub_op.first == peek) {
                    popNextChar();
                    cursorPos.column++;
                    push(sub_op.second, TokenType::OPERATOR, cursorPos);
                    return true;
                }
            }
        }
    }

    return false;
}