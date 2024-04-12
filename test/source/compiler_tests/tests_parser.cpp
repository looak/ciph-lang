#include <gtest/gtest.h>

#include "parser.hpp"
#include "ast.hpp"

TEST(ParserTest, SimpleAddExpression)
{
	// setup
	std::string code("1 + 2");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	ASSERT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto binaryExpression = static_cast<ASTBinaryExpressionNode*>(result->readStatements()[0]);
	EXPECT_EQ(binaryExpression->readOperator() , OperatorType::ADDITION);
}

TEST(ParserTest, OrderOfOperations_MulBeforeAdd)
{
	// setup
	std::string code("2 * 2 + 3");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto addExpression = static_cast<const ASTBinaryExpressionNode*>(result->readStatements()[0]);
	EXPECT_EQ(addExpression->readOperator() , OperatorType::ADDITION);

	auto left = addExpression->readLeft();
	EXPECT_EQ(left->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto mulExpression = static_cast<const ASTBinaryExpressionNode*>(left);
	EXPECT_EQ(mulExpression->readOperator() , OperatorType::MULTIPLICATION);
}

TEST(ParserTest, OrderOfOperations_ParenthaseseBeforeMul)
{
	// setup
	std::string code("2 * (2 + 3)");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto mulExpression = static_cast<const ASTBinaryExpressionNode*>(result->readStatements()[0]);
	EXPECT_EQ(mulExpression->readOperator() , OperatorType::MULTIPLICATION);

	auto right = mulExpression->readRight();
	EXPECT_EQ(right->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto addExpression = static_cast<const ASTBinaryExpressionNode*>(right);
	EXPECT_EQ(addExpression->readOperator() , OperatorType::ADDITION);
}

TEST(ParserTest, ReturnStatement_DefaultNumericNode_ValueZero)
{
	// setup
	std::string code("return");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);	
	auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);
	
	EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
	auto numericNode = static_cast<const ASTNumericLiteralNode*>(returnNode->readExpression());
	EXPECT_EQ(numericNode->readValue(), 0);
}

TEST(ParserTest, ReturnStatement_NumericNodeGivenValue)
{
	// setup
	std::string code("return 5");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);	
	auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);
	
	EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
	auto numericNode = static_cast<const ASTNumericLiteralNode*>(returnNode->readExpression());
	EXPECT_EQ(numericNode->readValue(), 5);
}

TEST(ParserTest, ReturnStatement_NumericNodeExpression)
{
	// setup
	std::string code("return 5 + 2");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);	
	auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);
	
	EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::BINARY_EXPRESSION);	
}