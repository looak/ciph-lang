#include <ciph/compiler/lexer.hpp>
#include <ciph/shared/shared_defines.hpp>
#include <ciph/compiler/lexer_defines.hpp>

using namespace ciph;


std::string_view
Token::readStringValue() const {
    return m_stringValue;
}

OperatorType
Token::readOperatorValue() const {
    if (m_type != TokenType::OPERATOR)
        return OperatorType::UNKNOWN;

    return m_operatorValue;
}

TokenType
Token::readType() const {
    return m_type;
}

Position
Token::readPosition() const {
    return m_position;
}

uint32_t
Token::readLength() const {
    return m_length;
}

Location
Token::readLocation() const {
    Position end = m_position;
    end.column += m_length;
    return Location{.span = {.start = m_position, .end = end}};
}

uint32_t
Token::readIndex() const {
    return m_index;
}

Lexer::Lexer(std::string_view input)
    : m_input(input)
    , m_position(0)
    , m_tokens()
    , m_performedAnalysis(false)
    , m_index(0) {
}

void
Lexer::push(std::variant<std::string_view, OperatorType> value, TokenType type, Position& position, uint32_t length) {
    if (std::holds_alternative<std::string_view>(value)) {
        m_tokens.emplace_back(std::get<std::string_view>(value), type, position, length, m_index);
    }
    else {
        m_tokens.emplace_back(std::get<OperatorType>(value), type, position, length, m_index);
    }
    
    position.column++;
    m_index++;
}

void
Lexer::skipWhiteSpaces(Position& cursorPos) {
    bool done = false;
    do {
        // m_input is a string_view: unlike std::string, it has no '\0' at
        // size(), so stop before indexing past the end.
        if (m_position >= m_input.size()) {
            done = true;
        }
        else if (m_input[m_position] == '\n') {
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
        else if (m_input[m_position] == '\r') {
            // Windows line endings are "\r\n"; the '\n' that follows does the
            // line counting.
            m_position++;
        }
        else if (m_input.compare(m_position, 2, "//") == 0) {
            // A comment runs to the end of the line. The '\n' is left for the
            // branch above, so lines are still counted.
            while (m_position < m_input.size() && m_input[m_position] != '\n') {
                m_position++;
            }
        }
        else {
            done = true;
        }

    } while (!done);
}

std::pair<bool, ErrorCode>
Lexer::lex() {
    if (m_performedAnalysis)
        return {false, ErrorCode::LEXER_PERFORMED_ANALYSIS};
    Position cursorPosition = {1, 1};

    while (m_position < m_input.size()) {
        TokenType type = TokenType::UNKNOWN;
        skipWhiteSpaces(cursorPosition);
        char cursor = popNextChar();
        if (cursor == ';') {
            push(";", TokenType::SEMICOLON, cursorPosition, 1);
            continue;
        }
        else if (cursor == ',') {
            push(",", TokenType::COMMA, cursorPosition, 1);
            continue;
        }
        else if (cursor == ':') {
            push(":", TokenType::COLON, cursorPosition, 1);
            continue;
        }
        else if (cursor == '-' && peekNextChar() == '>') {
            // Checked before the operators, which would read '-' and '>'
            // as two tokens.
            popNextChar();
            push("->", TokenType::ARROW, cursorPosition, 2);
            cursorPosition.column++; // two characters wide
            continue;
        }
        else if (cursor == '(') {
            push("(", TokenType::OPEN_PAREN, cursorPosition, 1);
            continue;
        }
        else if (cursor == ')') {
            push(")", TokenType::CLOSE_PAREN, cursorPosition, 1);
            continue;
        }
        else if (cursor == '[') {
            push("[", TokenType::OPEN_BRACKET, cursorPosition, 1);
            continue;
        }
        else if (cursor == ']') {
            push("]", TokenType::CLOSE_BRACKET, cursorPosition, 1);
            continue;
        }
        else if (cursor == '{') {
            push("{", TokenType::OPEN_BRACE, cursorPosition, 1);
            continue;
        }
        else if (cursor == '}') {
            push("}", TokenType::CLOSE_BRACE, cursorPosition, 1);
            continue;
        }
        else if (cursor == '\0') // eof
            break;

        if (isdigit(cursor)) {
            const std::size_t first = m_position - 1; // `cursor` was already popped
            uint32_t start = cursorPosition.column;
            do {
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
            std::string_view value = m_input.substr(first, m_position - first);
            push(value, type, cursorPosition, static_cast<uint32_t>(value.size()));
            cursorPosition.column += difference;
            continue;
        }

        // Identifiers and keywords start with a letter or '_' and continue
        // with letters, digits and '_', so `gcd_loop` and `i16` are one token.
        if (isalpha(cursor) || cursor == '_') {
            const std::size_t first = m_position - 1; // `cursor` was already popped
            uint32_t start = cursorPosition.column;
            do {                
                type = TokenType::IDENTIFIER;

                cursor = popNextChar();
                cursorPosition.column++;

            } while (isalnum(cursor) || cursor == '_');

            m_position--;
            cursorPosition.column--;
            std::string_view value = m_input.substr(first, m_position - first);

            if (s_keywords.find(value) != s_keywords.end()) {
                type = s_keywords.at(value);
            }


            // idea here is to set the cursor to the beginning of the digit, so that when it's referenced later we're
            // pointing the user to the beginning of the digit.
            uint32_t difference = cursorPosition.column - start;
            cursorPosition.column -= difference;
            push(value, type, cursorPosition, static_cast<uint32_t>(value.size()));
            cursorPosition.column += difference;

            continue;
        }

        if (identifyOperator(cursor, cursorPosition) == true) {
            continue;
        }

        // if we reach this without hitting a continue, we have an unknown character
        m_performedAnalysis = true;
        m_hasErrors = true;
        return {false, ErrorCode::LEXER_UNKNOWN_CHARACTER};
    }

    m_performedAnalysis = true;
    push("eof", TokenType::END_OF_FILE, cursorPosition, 0);
    return {true, ErrorCode::NO_ERR};
}

// Returns '\0' once the input is used up, which the lexing loops treat as the
// end. m_position still advances, because those loops step back one character
// after reading past the end of a number or name.
char
Lexer::popNextChar() {
    const char c = m_position < m_input.size() ? m_input[m_position] : '\0';
    ++m_position;
    return c;
}

char
Lexer::peekNextChar() {
    if (m_position >= m_input.size())
        return '\0';

    return m_input[m_position];
}

bool
Lexer::isOperator(char cursor) {
    if (cursor == '\0')
        return false;

    for (auto& op : s_operators) {
        if (op.first == cursor)
            return true;
    }
    return false;
}

bool
Lexer::identifyOperator(char cursor, Position& cursorPos) {
    for (auto& op : s_operators) {
        if (op.first == cursor) {
            char peek = peekNextChar();

            if (isOperator(peek) == false) {
                push(op.second[0].second, TokenType::OPERATOR, cursorPos, 1);
                return true;
            }

            for (auto& sub_op : op.second) {
                if (sub_op.first == peek) {
                    popNextChar();
                    // The token starts at the first of its two characters.
                    push(sub_op.second, TokenType::OPERATOR, cursorPos, 2);
                    cursorPos.column++;
                    return true;
                }
            }
        }
    }

    return false;
}