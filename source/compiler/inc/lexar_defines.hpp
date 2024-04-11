#pragma once
#include <shared_defines.hpp>

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