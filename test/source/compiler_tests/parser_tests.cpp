#include <gtest/gtest.h>

#include "parser.hpp"
#include "ast.hpp"

TEST(ParserTests, SimpleAddExpression)
{
	// setup
	std::string code("1 + 2");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
}
