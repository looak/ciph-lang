#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>

#include "lexar_defines.hpp"


class Token
{
 public:
	Token(const std::string& value, TokenType type);
	Token(const OperatorType& value, TokenType type);
	~Token() = default;

    Token(const Token& other) = default;

	std::string		readValue() const;
	OperatorType	readOperator() const;
	TokenType		readType() const;

	Token operator=(const Token& other)
	{
		m_value = other.m_value;
		m_type = other.m_type;
		return *this;
	}

	bool hasValue() const;

 private:
	std::variant<std::string, OperatorType> m_value;	
	TokenType m_type;
};

class Lexar
{
 public:
	explicit Lexar(const std::string& input);
	~Lexar() = default;

	Token peekNextToken();
	Token popNextToken();    
	//std::vector<std::string> getTokens() const;

private:
	char popNextChar();
	char peekNextChar();

	void skipWhiteSpaces();

	std::optional<Token> identifyOperator(char cursor);
	bool isOperator(char cursor);

	std::string m_input;
	uint32_t m_position;
	Token m_peek;

};

