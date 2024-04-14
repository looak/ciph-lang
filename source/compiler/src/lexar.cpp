#include "lexar.hpp"
#include "lexar_defines.hpp"

Token::Token(const std::string& value, TokenType type) :
	m_value(value),
	m_type(type)
{
}

Token::Token(const OperatorType& value, TokenType type) :
	m_value(value),
	m_type(type)
{
}

std::string Token::readValue() const
{
	return std::get<std::string>(m_value);
}

OperatorType Token::readOperator() const
{
	if (m_type != TokenType::OPERATOR)
		return OperatorType::UNKNOWN;
		
	return std::get<OperatorType>(m_value);
}

TokenType Token::readType() const
{
	return m_type;
}

bool Token::hasValue() const
{
	return m_type != TokenType::UNKNOWN;
}

Lexar::Lexar(const std::string& input) : 
	m_input(input),
	m_position(0),
    m_peek("", TokenType::UNKNOWN)
{
}

bool Lexar::popOperator(OperatorType op)
{
	Token token = pop();
	if (token.readType() != TokenType::OPERATOR)
		return false;
	if (token.readOperator() != op)
		return false;

	return true;
}

bool Lexar::expect(TokenType type)
{
	Token token = pop();
	if (token.readType() != type)
		return false;

	return true;
}

Token Lexar::peek()
{
	if (m_peek.hasValue())
		return m_peek;

	m_peek = pop();
    return m_peek;
}

Token Lexar::pop()
{
	if (m_peek.hasValue())
	{
		Token token = m_peek;
		m_peek = Token("", TokenType::UNKNOWN); // reset peek
		return token;
	}

	std::string value = "";
	TokenType type = TokenType::UNKNOWN;

	skipWhiteSpaces();

	while (m_position < m_input.size())
	{
		char cursor = popNextChar();
		if (cursor == '(')
			return Token("(", TokenType::OPEN_PAREN);
		else if (cursor == ')')
			return Token(")", TokenType::CLOSE_PAREN);
		else if (cursor == '[')
			return Token("[", TokenType::OPEN_BRACKET);
		else if (cursor == ']')
			return Token("]", TokenType::CLOSE_BRACKET);
		else if (cursor == '{')
			return Token("{", TokenType::OPEN_BRACE);
		else if (cursor == '}')
			return Token("}", TokenType::CLOSE_BRACE);

		if (isdigit(cursor)) {

            do {
                value += cursor;
                type = TokenType::NUMBER;

                cursor = popNextChar();
                
            } while (isdigit(cursor));

			// move position back one step since we read past the last digit.
			m_position--;

			return Token(value, type);
        }

		if (isalpha(cursor)) {
			do {
				value += cursor;
				type = TokenType::IDENTIFIER;

				cursor = popNextChar();

			} while (isalpha(cursor));

			m_position--;
						
			if (s_keywords.find(value) != s_keywords.end())
				return Token(value, s_keywords[value]);

			return Token(value, type);
		}

		auto op = identifyOperator(cursor);
		if (op.has_value())
            return op.value();
	}

	return Token("eof", TokenType::END_OF_FILE);
}

char Lexar::popNextChar()
{    
	return m_input[m_position++];
}

char Lexar::peekNextChar()
{
	if (m_position >= m_input.size())
		return '\0';

	return m_input[m_position];
}

void Lexar::skipWhiteSpaces()
{
	while (m_input[m_position] == ' ')
	{
		m_position++;
	}
}

bool Lexar::isOperator(char cursor)
{
	if (cursor == '\0')
		return false;

	for (auto& op : s_operators)
	{
		if (op.first == cursor)
			return true;
	}
	return false;
}

std::optional<Token>
Lexar::identifyOperator(char cursor)
{
	for (auto& op : s_operators)
	{
		if (op.first == cursor)
		{
			char peek = peekNextChar();

			if (isOperator(peek) == false)
                return Token(op.second[0].second, TokenType::OPERATOR);

			for (auto& sub_op : op.second)
			{
				if (sub_op.first == peek)
				{
					popNextChar();
                    return Token(sub_op.second, TokenType::OPERATOR);
				}
			}
		}
	}

	return {};
}