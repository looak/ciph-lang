#include <gtest/gtest.h>
#include "lexar.hpp"

TEST(LexarTest, PrimitiveTokens) 
{
    // setup
    Lexar lx("12345");
    
    // do
    auto token = lx.popNextToken();

    // validate
    EXPECT_EQ(token.readValue(), "12345");
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
}

bool
isOperator(const Token& token, OperatorType type)
{
    return token.readType() == TokenType::OPERATOR && token.readOperator() == type;
}

bool
OperatorTester(std::string input, OperatorType type)
{
	// setup
	Lexar lx(input);

	// do
	auto token = lx.popNextToken();

	// validate
	return isOperator(token, type);
}

TEST(LexarTest, IdentifyOperators)
{
    std::vector<std::pair<std::string, OperatorType>> operatorsToTest = {
		{"=", OperatorType::ASSIGNMENT},
		{"==", OperatorType::EQUAL},
		{"+", OperatorType::ADDITION},
		{"+=", OperatorType::ADDITION_ASSIGNMENT},
		{"++", OperatorType::INCREMENT},
		{"-", OperatorType::SUBTRACTION},
		{"-=", OperatorType::SUBTRACTION_ASSIGNMENT},
		{"--", OperatorType::DECREMENT},
	};

	for (auto& op : operatorsToTest)
	{
        EXPECT_TRUE(OperatorTester(op.first, op.second));
	}
}

TEST(LexarTest, SimpleAddExpression)
{
	// setup
	Lexar lx("1 +2");

	// do
	auto token1 = lx.popNextToken();
	auto token2 = lx.popNextToken();
	auto token3 = lx.popNextToken();

	// validate
	EXPECT_EQ(token1.readValue(), "1");
	EXPECT_EQ(token1.readType(), TokenType::NUMBER);

	EXPECT_TRUE(isOperator(token2, OperatorType::ADDITION));

	EXPECT_EQ(token3.readValue(), "2");
	EXPECT_EQ(token3.readType(), TokenType::NUMBER);
}

TEST(LexarTest, SimpleAssignmentExpression)
{
	Lexar lx("myVar = 10 + 5 - (3 * 2)");
	
	std::vector<Token> tokens;
	bool eofOrError = false;
	do {
		auto token = lx.popNextToken();
		tokens.push_back(token);
		if (token.readType() == TokenType::END_OF_FILE)
			eofOrError = true;
		else if (token.readType() == TokenType::UNKNOWN)
			eofOrError = true;
	} while (!eofOrError);

	EXPECT_EQ(tokens[0].readType(), TokenType::IDENTIFIER);
	EXPECT_EQ(tokens[0].readValue(), "myVar");	
	EXPECT_TRUE(isOperator(tokens[1], OperatorType::ASSIGNMENT));	
	EXPECT_EQ(tokens[2].readType(), TokenType::NUMBER);
	EXPECT_EQ(tokens[2].readValue(), "10");	
	EXPECT_TRUE(isOperator(tokens[3], OperatorType::ADDITION));	
	EXPECT_EQ(tokens[4].readType(), TokenType::NUMBER);
	EXPECT_EQ(tokens[4].readValue(), "5");	
	EXPECT_TRUE(isOperator(tokens[5], OperatorType::SUBTRACTION));	
	EXPECT_EQ(tokens[6].readType(), TokenType::OPEN_PAREN);
	EXPECT_EQ(tokens[6].readValue(), "(");
	EXPECT_EQ(tokens[7].readType(), TokenType::NUMBER);
	EXPECT_EQ(tokens[7].readValue(), "3");	
	EXPECT_TRUE(isOperator(tokens[8], OperatorType::MULTIPLICATION));	
	EXPECT_EQ(tokens[9].readType(), TokenType::NUMBER);
	EXPECT_EQ(tokens[9].readValue(), "2");
	EXPECT_EQ(tokens[10].readType(), TokenType::CLOSE_PAREN);
	EXPECT_EQ(tokens[10].readValue(), ")");
	EXPECT_EQ(tokens[11].readType(), TokenType::END_OF_FILE);
}

TEST(LexarTest, KeywordsIdentification)
{
	std::vector<std::pair<std::string, TokenType>> keywordsToTest = {
		{"var", TokenType::VAR},
		/*{"if", TokenType::IF},
		{"else", TokenType::ELSE},
		{"while", TokenType::WHILE},
		{"for", TokenType::FOR},
		{"return", TokenType::RETURN},
		{"break", TokenType::BREAK},
		{"continue", TokenType::CONTINUE},
		{"function", TokenType::FUNCTION},
		{"true", TokenType::TRUE},*/
	};

	for (auto& kw : keywordsToTest)
	{
		Lexar lx(kw.first);
		auto token = lx.popNextToken();
		EXPECT_EQ(token.readType(), kw.second);
	}
}

TEST(LexarTest, IdentifierLiteral)
{
	Lexar lx("myVar");
	auto token = lx.popNextToken();
	EXPECT_EQ(token.readValue(), "myVar");
	EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
}