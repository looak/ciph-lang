#include "lexar.hpp"

#include <map>

#include "lexar_defines.hpp"

Token::Token(const std::string& value, CIPHTokenType type)
    : m_value(value)
    , m_type(type) {
}

Token::Token(const OperatorType& value, CIPHTokenType type)
    : m_value(value)
    , m_type(type) {
}

std::string Token::readValue() const {
    return std::get<std::string>(m_value);
}

OperatorType Token::readOperator() const {
    if (m_type != CIPHTokenType::OPERATOR)
        return OperatorType::UNKNOWN;

    return std::get<OperatorType>(m_value);
}

CIPHTokenType Token::readType() const {
    return m_type;
}

bool Token::hasValue() const {
    return m_type != CIPHTokenType::UNKNOWN;
}

Lexar::Lexar(const std::string& input)
    : m_input(input)
    , m_position(0)
    , m_peek("", CIPHTokenType::UNKNOWN)
    , m_tokens()
    , m_performedAnalysis(false) {
}

LexarResult Lexar::lex() {
    if (m_performedAnalysis)
        return {false, ErrorCode::CIPH_LEXAR_PERFORMED_ANALYSIS};
    std::string value = "";
    CIPHTokenType type = CIPHTokenType::UNKNOWN;

    skipWhiteSpaces();

    while (m_position < m_input.size()) {
        char cursor = popNextChar();
        if (cursor == '(') {
            m_tokens.emplace_back("(", CIPHTokenType::OPEN_PAREN);
            continue;
        }
        else if (cursor == ')') {
            m_tokens.emplace_back(")", CIPHTokenType::CLOSE_PAREN);
            continue;
        }
        else if (cursor == '[') {
            m_tokens.emplace_back("[", CIPHTokenType::OPEN_BRACKET);
            continue;
        }
        else if (cursor == ']') {
            m_tokens.emplace_back("]", CIPHTokenType::CLOSE_BRACKET);
            continue;
        }
        else if (cursor == '{') {
            m_tokens.emplace_back("{", CIPHTokenType::OPEN_BRACE);
            continue;
        }
        else if (cursor == '}') {
            m_tokens.emplace_back("}", CIPHTokenType::CLOSE_BRACE);
            continue;
        }

        if (isdigit(cursor)) {
            do {
                value += cursor;
                type = CIPHTokenType::NUMBER;

                cursor = popNextChar();

            } while (isdigit(cursor));

            // move position back one step since we read past the last digit.
            m_position--;

            m_tokens.emplace_back(value, type);
            continue;
        }

        if (isalpha(cursor)) {
            do {
                value += cursor;
                type = CIPHTokenType::IDENTIFIER;

                cursor = popNextChar();

            } while (isalpha(cursor));

            m_position--;

            if (identifyKeyword(value) != CIPHTokenType::UNKNOWN) {
                m_tokens.emplace_back(value, identifyKeyword(value));
                continue;
            }

            m_tokens.emplace_back(value, type);
            continue;
        }

        auto op = identifyOperator(cursor);
        if (op.has_value()) {
            m_tokens.push_back(op.value());
            continue;
        }

        // if we reach this without hitting a continue, we have an unknown character
        return {false, ErrorCode::CIPH_LEXAR_UNKNOWN_CHARACTER};
    }

    // reset position, we'll reuse this variable when we pop & peek the tokens in our vector.
    m_position = 0;
    m_performedAnalysis = true;
    m_tokens.emplace_back("eof", CIPHTokenType::END_OF_FILE);
    return {true, ErrorCode::CIPH_NO_ERROR};
}

bool Lexar::popOperator(OperatorType op) {
    Token token = pop();
    if (token.readType() != CIPHTokenType::OPERATOR)
        return false;
    if (token.readOperator() != op)
        return false;

    return true;
}

bool Lexar::expect(CIPHTokenType type) {
    Token token = pop();
    if (token.readType() != type)
        return false;

    return true;
}

Token Lexar::peek() {
    if (m_peek.hasValue())
        return m_peek;

    m_peek = pop();
    return m_peek;
}

Token Lexar::pop() {
    if (m_peek.hasValue()) {
        Token token = m_peek;
        m_peek = Token("", CIPHTokenType::UNKNOWN); // reset peek
        return token;
    }
}

char Lexar::popNextChar() {
    return m_input[m_position++];
}

char Lexar::peekNextChar() {
    if (m_position >= m_input.size())
        return '\0';

    return m_input[m_position];
}

void Lexar::skipWhiteSpaces() {
    while (m_input[m_position] == ' ') {
        m_position++;
    }
}

bool Lexar::isOperator(char cursor) {
    if (cursor == '\0')
        return false;

    for (auto& op : s_operators) {
        if (op.first == cursor)
            return true;
    }
    return false;
}

std::optional<Token> Lexar::identifyOperator(char cursor) {
    for (auto& op : s_operators) {
        if (op.first == cursor) {
            char peek = peekNextChar();

            if (isOperator(peek) == false)
                return Token(op.second[0].second, CIPHTokenType::OPERATOR);

            for (auto& sub_op : op.second) {
                if (sub_op.first == peek) {
                    popNextChar();
                    return Token(sub_op.second, CIPHTokenType::OPERATOR);
                }
            }
        }
    }

    return {};
}

CIPHTokenType Lexar::identifyKeyword(const std::string& value) {
    static const std::map<std::string, CIPHTokenType> s_keywords = {
        {"let", CIPHTokenType::LET}, {"return", CIPHTokenType::RETURN} /*,
                                  {"if", CIPHTokenType::IF},
                                  {"else", CIPHTokenType::ELSE},
                                  {"while", CIPHTokenType::WHILE},
                                  {"for", CIPHTokenType::FOR},
                                  {"return", CIPHTokenType::RETURN},
                                  {"break", CIPHTokenType::BREAK},
                                  {"continue", CIPHTokenType::CONTINUE},
                                  {"function", CIPHTokenType::FUNCTION},
                                  {"true", CIPHTokenType::TRUE},
                                  {"false", CIPHTokenType::FALSE},
                                  {"null", CIPHTokenType::NULL}*/
    };
    auto token = s_keywords.find(value);
    if (token != s_keywords.end()) {
        return token->second;
    }

    return CIPHTokenType::UNKNOWN;
}