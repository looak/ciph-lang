#pragma once

#include <map>

enum class TokenType {
    // literals
	NUMBER,     
    IDENTIFIER,

	// keywords
	VAR,

	/*
    STRING,
    CHARACTER,
    BOOLEAN,*/

	// scope and operators
    OPERATOR,
	OPEN_PAREN,
	CLOSE_PAREN,
	OPEN_BRACE,
	CLOSE_BRACE,
	OPEN_BRACKET,
	CLOSE_BRACKET,

	END_OF_FILE,
    UNKNOWN
};

enum class OperatorType : uint8_t {
	ADDITION,
	SUBTRACTION,
	MULTIPLICATION,
	DIVISION,
	MODULUS,
	EQUAL,
	NOT,
	LESS,
	GREATER,
	AND,
	OR,
	XOR,
	NOT_EQUAL,
	LESS_EQUAL,
	GREATER_EQUAL,
	AND_AND,
	OR_OR,
	LEFT_SHIFT,
	RIGHT_SHIFT,
	ASSIGNMENT,
	ADDITION_ASSIGNMENT,
	SUBTRACTION_ASSIGNMENT,
	MULTIPLICATION_ASSIGNMENT,
	DIVISION_ASSIGNMENT,
	MODULUS_ASSIGNMENT,
	LEFT_SHIFT_ASSIGNMENT,
	RIGHT_SHIFT_ASSIGNMENT,
	AND_ASSIGNMENT,
	OR_ASSIGNMENT,
	XOR_ASSIGNMENT,
	INCREMENT,
	DECREMENT,
	UNKNOWN
};


typedef std::vector<std::pair<char, std::vector<std::pair<char, OperatorType>>>> OperatorMap;
static OperatorMap
generateOperatorMap()
{
    OperatorMap map = {
        {'=', {{'\0', OperatorType::ASSIGNMENT}, {'=', OperatorType::EQUAL}}},
        {'+', {{'\0', OperatorType::ADDITION}, {'+', OperatorType::INCREMENT}, {'=', OperatorType::ADDITION_ASSIGNMENT}}},
        {'-', {{'\0', OperatorType::SUBTRACTION}, {'-', OperatorType::DECREMENT}, {'=', OperatorType::SUBTRACTION_ASSIGNMENT}}},
		{'*', {{'\0', OperatorType::MULTIPLICATION}, {'=', OperatorType::MULTIPLICATION_ASSIGNMENT}}},
		{'/', {{'\0', OperatorType::DIVISION}, {'=', OperatorType::DIVISION_ASSIGNMENT}}},
    };

	return map;
}

static OperatorMap s_operators = generateOperatorMap();
static std::map<std::string, TokenType> s_keywords = {
	{"var", TokenType::VAR}/*,
	{"if", TokenType::IF},
	{"else", TokenType::ELSE},
	{"while", TokenType::WHILE},
	{"for", TokenType::FOR},
	{"return", TokenType::RETURN},
	{"break", TokenType::BREAK},
	{"continue", TokenType::CONTINUE},
	{"function", TokenType::FUNCTION},
	{"true", TokenType::TRUE},
	{"false", TokenType::FALSE},
	{"null", TokenType::NULL}*/
};